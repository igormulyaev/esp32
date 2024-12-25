#include "HttpsClient.hpp"

#include "esp_log.h"
#include "esp_tls.h"

// ----------------------------------------------------------------------------------
void HttpsClient :: Init (const char * url)
{
    ESP_LOGD (TAG, "Begin Init");

    esp_http_client_config_t cfg = 
    {
        .url = url,
        .cert_pem = certPem,
        .method = HTTP_METHOD_GET,
        .event_handler = EventHandler,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .user_data = this,
        .skip_cert_common_name_check = false,
    };

    clientHandle = esp_http_client_init (&cfg);

    if (clientHandle == NULL)
    {
        ESP_LOGE (TAG, "Init failed");
    }
    isMethodGet = true;
    
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
esp_err_t HttpsClient :: SetHeader (const char *key, const char *value)
{
    ESP_LOGD (TAG, "Begin SetHeader");

    esp_err_t rc = esp_http_client_set_header (clientHandle, key, value);

    if (rc != ESP_OK)
    {
        ESP_LOGE (TAG, "SetHeader error");
    }
    
    ESP_LOGD (TAG, "End SetHeader");
    
    return rc;
}

// ----------------------------------------------------------------------------------
esp_err_t HttpsClient :: SetPostData (const char * data, int len)
{
    ESP_LOGD (TAG, "Begin SetPostData");

    esp_err_t rc = esp_http_client_set_post_field (clientHandle, data, len);

    if (rc != ESP_OK)
    {
        ESP_LOGE (TAG, "SetPostData error");
    }
    
    ESP_LOGD (TAG, "End SetPostData");
    
    return rc;
}

// ----------------------------------------------------------------------------------
esp_err_t HttpsClient :: SetTimeoutMs (int timeoutMs)
{
    ESP_LOGD (TAG, "Begin SetTimeoutMs %d", timeoutMs);

    esp_err_t rc = esp_http_client_set_timeout_ms (clientHandle, timeoutMs);

    if (rc != ESP_OK)
    {
        ESP_LOGE (TAG, "SetTimeoutMs error");
    }
    
    ESP_LOGD (TAG, "End SetTimeoutMs");
    
    return rc;
}

// ----------------------------------------------------------------------------------
esp_err_t HttpsClient :: SetMethod(esp_http_client_method_t method)
{
    ESP_LOGD (TAG, "Begin SetMethod");

    esp_err_t rc = esp_http_client_set_method (clientHandle, method);

    if (rc != ESP_OK)
    {
        ESP_LOGE (TAG, "SetMethod error");
    }
    else
    {
        isMethodGet = (method == HTTP_METHOD_GET);
    }
    
    ESP_LOGD (TAG, "End SetMethod");
    
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