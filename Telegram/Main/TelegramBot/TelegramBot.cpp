#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

#include "TelegramBot.hpp"
#include "StaticObjects.hpp"

#include "esp_err.h"
#include "cJSON.h"
#include <cstring>

// -----------------------------------------------------------------------
TelegramBot :: TelegramBot() : 
    state (tgStart)
    , client (cert_pem_start)
{
}

// -----------------------------------------------------------------------
void TelegramBot :: Init (const char *id, const char *key)
{
    ESP_LOGI (TAG, "Begin Init");
    
    basicUrl = "https://api.telegram.org/bot";
    basicUrl.append (id);
    basicUrl.append (":");
    basicUrl.append (key);
    basicUrl.append ("/");

    ESP_LOGD (TAG, "End Init");
}

// -----------------------------------------------------------------------
void TelegramBot :: Process()
{
    ESP_LOGI (TAG, "Begin Process");
    
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
            case tgUpdate:
                state = Update();
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
    ESP_LOGI (TAG, "Begin Start");
    
    client.Init (basicUrl.c_str());
    
    ESP_LOGD (TAG, "End Start");
    
    return tgGetMe;
}

// -----------------------------------------------------------------------
TelegramBot :: States TelegramBot :: GetMe()
{
    ESP_LOGI (TAG, "Begin GetMe");

    fullUrl = basicUrl;
    fullUrl.append ("getMe");

    if (client.SetUrl (fullUrl.c_str()) != ESP_OK
        || client.Perform() != ESP_OK) 
    {
        return tgStop;
    }

    ESP_LOGD (TAG, "Data received: \"%s\"", client.result.c_str());

    bool res = false;

    cJSON * jRoot = cJSON_Parse (client.result.c_str());

    cJSON * jOk = jRoot -> child;
    if (jOk != NULL
        && strcmp (jOk -> string, "ok") == 0
        && (jOk -> type & cJSON_True) != 0)
    {
        ESP_LOGI (TAG, "GetMe ok");

        std :: string newFirstname, newUsername;

        cJSON * jResult = jOk -> next;
        if (jResult != NULL)
        {
            for (cJSON * jEl = jResult -> child; jEl != NULL; jEl = jEl -> next)
            {
                if ((jEl -> type & cJSON_String) != 0)
                {
                    if (strcmp (jEl -> string, "first_name") == 0)
                    {
                        newFirstname = jEl -> valuestring;
                    }
                    else if (strcmp (jEl -> string, "username") == 0)
                    {
                        newUsername = jEl -> valuestring;
                    }
                }
            }
        }

        Firstname = newFirstname;
        Username = newUsername;

        ESP_LOGI (TAG, "Got bot params: Firstname = \"%s\", Username = \"%s\"", Firstname.c_str(), Username.c_str());
        res = true;
    }

    cJSON_Delete (jRoot);
    
    ESP_LOGD (TAG, "End GetMe");

    return res ? tgUpdate : tgStop;
}

// -----------------------------------------------------------------------
TelegramBot :: States TelegramBot :: Update()
{
    ESP_LOGI (TAG, "Begin Update");
    
    fullUrl = basicUrl;
    fullUrl.append ("getUpdates");

    if (client.SetUrl (fullUrl.c_str()) != ESP_OK
        || client.Perform() != ESP_OK) 
    {
        return tgStop;
    }

    ESP_LOGD (TAG, "Data received: \"%s\"", client.result.c_str());
    ESP_LOGD (TAG, "End Update");
    return tgStop;
}

// -----------------------------------------------------------------------
TelegramBot :: States TelegramBot :: Stop()
{
    ESP_LOGI (TAG, "Begin Stop");
    
    client.Cleanup();
    
    ESP_LOGD (TAG, "End Stop");
    
    return tgExit;
}

// -----------------------------------------------------------------------
const char * const TelegramBot :: TAG = "TelegramBot";