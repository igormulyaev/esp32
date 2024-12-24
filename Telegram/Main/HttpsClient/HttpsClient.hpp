#ifndef HTTPSCLIENT_HPP
#define HTTPSCLIENT_HPP

#include <string>

// -----------------------------------------------------------------------
struct esp_http_client;
typedef struct esp_http_client *esp_http_client_handle_t;

struct esp_http_client_event;
typedef struct esp_http_client_event esp_http_client_event_t;

typedef int esp_err_t;

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
        esp_err_t SetTimeoutMs (int timeoutMs);
        
        std :: string result;
        
    private:
        const char * certPem;

        esp_http_client_handle_t clientHandle = NULL;
        
        static esp_err_t EventHandler(esp_http_client_event_t *evt);

        static const char * const TAG;
};

#endif // HTTPSCLIENT_HPP