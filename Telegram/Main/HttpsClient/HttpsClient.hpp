#ifndef HTTPSCLIENT_HPP
#define HTTPSCLIENT_HPP

#include <string>
#include "esp_http_client.h"

// -----------------------------------------------------------------------
class HttpsClient
{
    public:
        HttpsClient (const char * certPem = NULL): certPem (certPem) {};
        ~HttpsClient() {};

        void Init (const char * url);
        esp_err_t Perform();
        void Cleanup();

        esp_err_t SetUrl (const char * url);
        esp_err_t SetHeader (const char *key, const char *value);
        esp_err_t SetPostData (const char * data, int len);
        esp_err_t SetTimeoutMs (int timeoutMs);

        esp_err_t SetMethodGet() { if (!isMethodGet) return SetMethod(HTTP_METHOD_GET); return ESP_OK; }
        esp_err_t SetMethodPost() { if (isMethodGet) return SetMethod(HTTP_METHOD_POST); return ESP_OK; }

        std :: string result;
        
    private:
        const char * certPem;

        esp_http_client_handle_t clientHandle = NULL;

        bool isMethodGet = true;
        esp_err_t SetMethod(esp_http_client_method_t method);
        
        static esp_err_t EventHandler(esp_http_client_event_t *evt);

        static const char * const TAG;
};

#endif // HTTPSCLIENT_HPP