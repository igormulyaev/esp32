#include "HttpUriHandlerBase.hpp"

// ----------------------------------------------------------------------------------
esp_err_t HttpUriHandlerBase :: HandlerStatic (httpd_req_t *r)
{
    HttpUriHandlerBase *hb = static_cast <HttpUriHandlerBase *> (r -> user_ctx);
    return hb -> Handler (r);
}