#include "HttpServerBase.hpp"
#include "HttpUriHandlerBase.hpp"
#include "esp_log.h"

// ----------------------------------------------------------------------------------
void HttpServerBase :: Start()
{
    ESP_LOGI (TAG, "Begin Start");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_ERROR_CHECK (httpd_start (&serv, &config));

    ESP_LOGI (TAG, "End Start");
}

// ----------------------------------------------------------------------------------
void HttpServerBase :: Stop()
{
    ESP_LOGI (TAG, "Begin Stop");

    ESP_ERROR_CHECK (httpd_stop (serv));
    serv = NULL;

    ESP_LOGI (TAG, "End Stop");
}

// ----------------------------------------------------------------------------------
void HttpServerBase :: RegisterUriHandler (
    HttpUriHandlerBase &uriHandler,
    const char *uri,
    httpd_method_t method 
)
{
    ESP_LOGI (TAG, "Begin Register");
    
    httpd_uri_t uriCfg 
    {
        .uri = uri,
        .method = method,
        .handler = uriHandler.HandlerStatic,
        .user_ctx = &uriHandler
    };

    ESP_ERROR_CHECK (httpd_register_uri_handler (serv, &uriCfg));

    ESP_LOGI (TAG, "End Register");
}

// ----------------------------------------------------------------------------------
const char * const HttpServerBase :: TAG = "HttpServerBase";