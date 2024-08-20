#ifndef HTTPURITEST_HPP
#define HTTPURITEST_HPP

#include "HttpUriHandlerBase.hpp"

class HttpUriTest: public HttpUriHandlerBase
{
    public:
        HttpUriTest(): HttpUriHandlerBase() {};
        virtual ~HttpUriTest() {};

    private:
        virtual esp_err_t Handler (httpd_req_t *r);

        static const char * const TAG;
};

#endif