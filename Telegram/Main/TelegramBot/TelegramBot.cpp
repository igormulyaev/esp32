#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

#include "TelegramBot.hpp"
#include "StaticObjects.hpp"
#include "AnswerParserGetMe.hpp"
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

    ESP_LOGD (TAG, "GetMe url = %s", fullUrl.c_str());

    if (client.SetUrl (fullUrl.c_str()) != ESP_OK 
        || client.SetMethodGet() != ESP_OK
        || client.Perform() != ESP_OK) 
    {
        ESP_LOGE (TAG, "SetUrl or Perform error");
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

    ESP_LOGD (TAG, "Update url = %s", fullUrl.c_str());

    if (client.SetUrl (fullUrl.c_str()) != ESP_OK
        || client.SetMethodGet() != ESP_OK
        || client.Perform() != ESP_OK) 
    {
        ESP_LOGE (TAG, "SetUrl or Perform error");
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
            
            ESP_LOGI (TAG
                , "Got message: uid = %lu, msg_id = %lu, date = %lu, text = \"%s\""
                , uid
                , parser.getMessageId()
                , parser.getDate()
                , parser.getText().c_str()
            );

            TgUser const & from = parser.getFrom();
            ESP_LOGI (TAG
                , "From: id = %llu, first_name = \"%s\", last_name = \"%s\", username = \"%s\", language_code = \"%s\""
                , from.id
                , from.first_name.c_str()
                , from.last_name.c_str()
                , from.username.c_str()
                , from.language_code.c_str()
            );

            TgChat const & chat = parser.getChat();
            ESP_LOGI (TAG, "Chat: id = %llu", chat.id);
            rc = state == tgReadMessage ? tgSendMessage : tgReadOldMessages;
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
TelegramBot :: States TelegramBot :: Send()
{
    ESP_LOGI (TAG, "Send");
    
    std::string fullUrl = basicUrl;
    fullUrl += "sendMessage";

    ESP_LOGD (TAG, "Send url = %s", fullUrl.c_str());

    std::string text = "{\"chat_id\":584216360, \"text\":\"Я бот посольского приказа\"}";

    ESP_LOGI (TAG, "Send text = \"%s\"", text.c_str());

    if (client.SetUrl (fullUrl.c_str()) != ESP_OK
        || client.SetMethodPost() != ESP_OK
        || client.SetHeader ("Content-Type", "application/json") != ESP_OK
        || client.SetPostData (text.c_str(), text.length()) != ESP_OK
        || client.Perform() != ESP_OK) 
    {
        ESP_LOGE (TAG, "SetUrl or Perform error");
        return tgStop;
    }

    ESP_LOGI (TAG, "Data received: \"%s\"", client.result.c_str());

    ESP_LOGD (TAG, "End Send");
    return tgReadMessage;
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
const char * const TelegramBot :: TAG = "TelegramBot";