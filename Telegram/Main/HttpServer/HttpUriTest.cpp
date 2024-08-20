#include "HttpUriTest.hpp"
#include "esp_log.h"

// ----------------------------------------------------------------------------------
esp_err_t HttpUriTest :: Handler (httpd_req_t *r)
 {
    ESP_LOGI (TAG, "Got request: \"%s\"", r -> uri);
    return httpd_resp_send(r, "<html> <head> </head> <body> Hello, World! </body> </html>", HTTPD_RESP_USE_STRLEN);
 }
 
// ----------------------------------------------------------------------------------
const char * const HttpUriTest :: TAG = "HttpUriTest";