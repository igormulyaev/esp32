#include "HttpsClient.hpp"

#include "esp_log.h"
#include "esp_tls.h"
#include "esp_http_client.h"

// ----------------------------------------------------------------------------------
void HttpsClient :: Init (const char * url)
{
    ESP_LOGD (TAG, "Begin Init");

    esp_http_client_config_t cfg = 
    {
        .url = url,
        .cert_pem = certPem,
        .event_handler = EventHandler,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .user_data = this,
    };

    clientHandle = esp_http_client_init (&cfg);

    if (clientHandle == NULL)
    {
        ESP_LOGE (TAG, "Init failed");
    }
    ESP_LOGD (TAG, "End Init");
}

// ----------------------------------------------------------------------------------
esp_err_t HttpsClient :: Perform()
{
    ESP_LOGD (TAG, "Begin Perform");

    result.clear();

    esp_err_t rc = esp_http_client_perform (clientHandle);
    
    ESP_LOGD (TAG, "End Perform");
    
    return rc;
}

// ----------------------------------------------------------------------------------
esp_err_t HttpsClient :: SetUrl (const char * url)
{
    ESP_LOGD (TAG, "Begin SetUrl \"%s\"", url);

    esp_err_t rc = esp_http_client_set_url (clientHandle, url);

    if (rc != ESP_OK)
    {
        ESP_LOGE (TAG, "SetUrl error");
    }
    
    ESP_LOGD (TAG, "End SetUrl");
    
    return rc;
}

// ----------------------------------------------------------------------------------
void HttpsClient :: Cleanup()
{
    ESP_LOGD (TAG, "Begin Cleanup");

    esp_http_client_cleanup (clientHandle);
    
    clientHandle = NULL;
    
    ESP_LOGD (TAG, "End Cleanup");
}

// ----------------------------------------------------------------------------------
esp_err_t HttpsClient :: EventHandler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE (TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD (TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD (TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD (TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            {
                ESP_LOGD (TAG, "HTTP_EVENT_ON_DATA, len = %d", evt -> data_len);
            
                auto * clntThis = static_cast <HttpsClient *> (evt -> user_data);
                auto & result = clntThis -> result;

                result.append (static_cast <const char *> (evt -> data), evt -> data_len);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD (TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            {
                ESP_LOGI (TAG, "HTTP_EVENT_DISCONNECTED");
                
                int mbedtls_err = 0;
                esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
                
                if (err != 0) 
                {
                    ESP_LOGI (TAG, "Last esp error code: 0x%x", err);
                    ESP_LOGI (TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
                }
            } 
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGI (TAG, "HTTP_EVENT_REDIRECT");
            break;
    }
    return ESP_OK;
}

// ----------------------------------------------------------------------------------
const char * const HttpsClient :: TAG = "HttpsClient";