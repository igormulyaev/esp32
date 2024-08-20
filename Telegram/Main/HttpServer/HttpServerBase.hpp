#ifndef HTTPSERVERBASE_HPP
#define HTTPSERVERBASE_HPP

#include "esp_http_server.h"

class HttpUriHandlerBase;

class HttpServerBase
{
    public:
        HttpServerBase() {};
        virtual ~HttpServerBase() {};

        virtual void Start();
        void Stop();

        void RegisterUriHandler (HttpUriHandlerBase &uriHandler
            , const char *uri
            , httpd_method_t method 
        );

    private:
        httpd_handle_t serv = NULL;

        static const char * const TAG;
};

#endif