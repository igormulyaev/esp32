#ifndef HTTPURIHANDLERBASE_HPP
#define HTTPURIHANDLERBASE_HPP

#include "esp_err.h"
#include "esp_http_server.h"

class HttpUriHandlerBase
{
    friend class HttpServerBase;

    public:
        HttpUriHandlerBase() {};
        virtual ~HttpUriHandlerBase() {};

    private:
        virtual esp_err_t Handler (httpd_req_t *r) = 0;
        static esp_err_t HandlerStatic (httpd_req_t *r);
};

#endif