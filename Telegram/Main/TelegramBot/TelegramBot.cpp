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
            case tgReadOldMessages:
            case tgReadMessage:
                state = Update();
                break;
            case tgAnswer:
                state = Answer();
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

    return isOk ? tgReadOldMessages : tgStop;
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

    AnswerParserUpdate parser(client.result);

    TelegramBot :: States rc = tgStop;
    if (parser.getIsOk())
    {
        ESP_LOGD (TAG, "Update ok");

        uint32_t uid = parser.getUpdateId();

        if (uid != 0)
        {
            lastMsgId = uid;
            
            message = parser.getMessage();
            rc = state == tgReadMessage ? tgAnswer : tgReadOldMessages;

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
                , message.from.language_code.c_str()
            );

            ESP_LOGI (TAG, "Chat: id = %llu", message.chat.id);
        }
        else
        {
            ESP_LOGI (TAG, "No messages");
            rc = tgReadMessage;
        }
    }
    
    ESP_LOGD (TAG, "End Update");
    return rc;
}
// -----------------------------------------------------------------------
TelegramBot :: States TelegramBot :: Answer()
{
    ESP_LOGI (TAG, "Answer");

    answerText = "Привет, ";
    answerText += message.from.first_name;
    answerText += "!\n";
    answerText += "Я бот посольского приказа.";

    ESP_LOGD (TAG, "End Answer");
    return tgSendMessage;
}

// -----------------------------------------------------------------------
TelegramBot :: States TelegramBot :: Send()
{
    ESP_LOGI (TAG, "Send");
    
    std::string fullUrl = basicUrl;
    fullUrl += "sendMessage";

    std::string jReply = "{\"chat_id\":";
    jReply += std::to_string (message.chat.id);
    jReply += ",\"text\":\"";
    jReply += answerText;
    jReply += "\"}";

    ESP_LOGI (TAG, "Send text = \"%s\"", jReply.c_str());

    if (PerformPost (fullUrl, jReply) != ESP_OK)
    {
        return tgStop;
    }

    ESP_LOGI (TAG, "Data received: \"%s\"", client.result.c_str());

    AnswerParserSend parser(client.result);

    ESP_LOGD (TAG, "End Send");

    return parser.getIsOk() ? tgReadMessage : tgStop;
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