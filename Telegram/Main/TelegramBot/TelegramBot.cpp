#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

#include "TelegramBot.hpp"
#include "StaticObjects.hpp"
#include "AnswerParserGetMe.hpp"
#include "AnswerParserSend.hpp"
#include "AnswerParserUpdate.hpp"

#include "esp_err.h"
#include "cJSON.h"
#include <cstring>

// -----------------------------------------------------------------------
TelegramBot :: TelegramBot() : 
    state (tgStart)
    , client (cert_pem_start)
    , lastMsgId (0)
{
}

// -----------------------------------------------------------------------
void TelegramBot :: Init (const char *id, const char *key)
{
    ESP_LOGI (TAG, "Init");
    
    basicUrl = "https://api.telegram.org/bot";
    basicUrl.append (id);
    basicUrl.append (":");
    basicUrl.append (key);
    basicUrl.append ("/");
}

// -----------------------------------------------------------------------
void TelegramBot :: Process()
{
    ESP_LOGD (TAG, "Begin Process");
    
    while (state != tgExit) 
    {
        switch (state)
        {
            case tgStart:
                state = Start();
                break;
            case tgGetMe:
                state = GetMe();
                break;
            case tgReadOldMessage:
            case tgReadMessage:
                state = Update();
                break;
            case tgStoreOldMessage:
                state = StoreOldMessage();
                break;
            case tgAnswerNewMessage:
                state = AnswerNewMessage();
                break;
            case tgAnswerOldMessages:
                state = AnswerOldMessages();
                break;
            case tgSendMessage:
                state = Send();
                break;            
            case tgStop:
                state = Stop();
                break;
            case tgExit:
                break;
        }
    }

    ESP_LOGD (TAG, "End Process");
}

// -----------------------------------------------------------------------
TelegramBot :: States TelegramBot :: Start()
{
    ESP_LOGI (TAG, "Start");
    
    client.Init (basicUrl.c_str());
    client.SetTimeoutMs (70000);
    
    return tgGetMe;
}

// -----------------------------------------------------------------------
TelegramBot :: States TelegramBot :: GetMe()
{
    ESP_LOGI (TAG, "GetMe");

    std::string fullUrl = basicUrl;
    fullUrl.append ("getMe");

    if (PerformGet (fullUrl) != ESP_OK)
    {
        return tgStop;
    }

    ESP_LOGD (TAG, "Data received: \"%s\"", client.result.c_str());

    AnswerParserGetMe parser(client.result);

    bool isOk = parser.getIsOk();
    if (isOk) 
    {
        Firstname = parser.getFirstname();
        Username = parser.getUsername();

        ESP_LOGI (TAG, "Got bot params: Firstname = \"%s\", Username = \"%s\"", Firstname.c_str(), Username.c_str());
    }
    ESP_LOGD (TAG, "End GetMe");

    return isOk ? tgReadOldMessage : tgStop;
}

// -----------------------------------------------------------------------
TelegramBot :: States TelegramBot :: Update()
{
    ESP_LOGI (TAG, "Update %s", state == tgReadMessage ? "message" : "old messages");
    
    std::string fullUrl = basicUrl;
    fullUrl += "getUpdates?limit=1";
    if (lastMsgId != 0)
    {
        fullUrl += "&offset=";
        fullUrl += std::to_string (lastMsgId + 1);
    }
    if (state == tgReadMessage)
    {
        fullUrl += "&timeout=60";
    }

    if (PerformGet (fullUrl) != ESP_OK)
    {
        return tgStop;
    }

    ESP_LOGD (TAG, "Data received: \"%s\"", client.result.c_str());

    AnswerParserUpdate parser(client.result, message);

    TelegramBot :: States rc = tgStop;
    if (parser.getIsOk())
    {
        ESP_LOGD (TAG, "Update ok");

        uint32_t uid = parser.getUpdateId();

        if (uid != 0)
        {
            lastMsgId = uid;
            
            rc = state == tgReadMessage ? tgAnswerNewMessage : tgStoreOldMessage;

            ESP_LOGI (TAG
                , "Got message: uid = %lu, msg_id = %llu, date = %llu, text = \"%s\""
                , uid
                , message.message_id
                , message.date
                , message.text.c_str()
            );

            ESP_LOGI (TAG
                , "From: id = %llu, first_name = \"%s\", last_name = \"%s\", username = \"%s\", language_code = \"%s\""
                , message.from.id
                , message.from.first_name.c_str()
                , message.from.last_name.c_str()
                , message.from.username.c_str()
                , message.from.language_code
            );

            ESP_LOGI (TAG, "Chat: id = %llu", message.chat.id);
        }
        else
        {
            ESP_LOGI (TAG, "No messages");
            rc = oldMessages != nullptr  ? tgAnswerOldMessages : tgReadMessage;
        }
    }
    
    ESP_LOGD (TAG, "End Update");
    return rc;
}
// -----------------------------------------------------------------------
TelegramBot :: States TelegramBot :: StoreOldMessage()
{
    ESP_LOGI (TAG, "StoreOldMessage");

    if (oldMessages == nullptr)
    {
        oldMessages = std::make_unique <TgOldMessages>();
    }

    size_t cnt = oldMessages -> count;
    if (cnt < TgOldMessages::maxUsers)
    {
        // Try to find the chat in the list, add the chat if it's not found
        size_t i = 0;
        for (; i < cnt; i++)
        {
            if (oldMessages -> messages[i].chat.id == message.chat.id)
            {
                break;
            }
        }
        if (i == cnt)
        {
            // The chat haven't been stored yet
            oldMessages -> messages[cnt].chat = message.chat;
            oldMessages -> messages[cnt].replyName = message.from.first_name.empty() ? message.from.username : message.from.first_name;
            oldMessages -> count = cnt + 1;
        }
    }
    ESP_LOGD (TAG, "End StoreOldMessage");
    return tgReadOldMessage;
}

// -----------------------------------------------------------------------
TelegramBot :: States TelegramBot :: AnswerNewMessage()
{
    ESP_LOGI (TAG, "AnswerNewMessage");

    answer.text = "Привет, ";
    answer.text += message.from.first_name;
    answer.text += "!\n";
    answer.text += "Commands: /start, /help, /test";
    answer.chat = message.chat;

    ESP_LOGD (TAG, "End AnswerNewMessage");
    return tgSendMessage;
}

// -----------------------------------------------------------------------
TelegramBot :: States TelegramBot :: AnswerOldMessages()
{
    ESP_LOGI (TAG, "AnswerOldMessages");

    TelegramBot :: States rc = tgSendMessage;

    size_t cnt = oldMessages -> count;
    if (cnt > 0)
    {
        answer.text = "Привет, ";
        answer.text += oldMessages -> messages[cnt - 1].replyName;
        answer.text += "!\n";
        answer.text += "Я снова здесь!\n";
        answer.chat = oldMessages -> messages[cnt - 1].chat;
        oldMessages -> count = cnt - 1;
    }
    else
    {
        oldMessages.reset();
        rc = tgReadMessage;
    }

    ESP_LOGD (TAG, "End AnswerOldMessages");
    return rc;
}

// -----------------------------------------------------------------------
TelegramBot :: States TelegramBot :: Send()
{
    ESP_LOGI (TAG, "Send");
    
    std::string fullUrl = basicUrl;
    fullUrl += "sendMessage";

    std::string jReply = "{\"chat_id\":";
    jReply += std::to_string (answer.chat.id);
    jReply += ",\"text\":\"";
    jReply += answer.text;
    jReply += "\"}";

    ESP_LOGI (TAG, "Send text = \"%s\"", jReply.c_str());

    if (PerformPost (fullUrl, jReply) != ESP_OK)
    {
        return tgStop;
    }

    ESP_LOGI (TAG, "Data received: \"%s\"", client.result.c_str());

    AnswerParserSend parser(client.result);

    ESP_LOGD (TAG, "End Send");

    return parser.getIsOk() ? (oldMessages != nullptr ? tgAnswerOldMessages : tgReadMessage) : tgStop;
}

// -----------------------------------------------------------------------
TelegramBot :: States TelegramBot :: Stop()
{
    ESP_LOGI (TAG, "Stop");
    
    client.Cleanup();
    
    ESP_LOGD (TAG, "End Stop");
    
    return tgExit;
}

// -----------------------------------------------------------------------
esp_err_t TelegramBot :: PerformGet (const std::string & url)
{
    ESP_LOGD (TAG, "Update url = %s", url.c_str());

    if (client.SetUrl (url.c_str()) != ESP_OK
        || client.SetMethodGet() != ESP_OK
        || client.Perform() != ESP_OK) 
    {
        ESP_LOGE (TAG, "PerformGet error");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

// -----------------------------------------------------------------------
esp_err_t TelegramBot :: PerformPost (const std::string & url, const std::string & data, const char * contentType)
{
    if (client.SetUrl (url.c_str()) != ESP_OK
        || client.SetMethodPost() != ESP_OK
        || client.SetHeader ("Content-Type", contentType) != ESP_OK
        || client.SetPostData (data.c_str(), data.length()) != ESP_OK
        || client.Perform() != ESP_OK) 
    {
        ESP_LOGE (TAG, "PerformPost error");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

// -----------------------------------------------------------------------
const char * const TelegramBot :: TAG = "TelegramBot";