#include "TelegramStructs.hpp"
#include "cJSON.h"
#include <cstring>
//#include <cstdlib>


bool TgUser :: populatefromJson(cJSON *jUser)
{
    bool rc = false;

    for (cJSON * jEl = jUser -> child; 
        jEl != NULL && jEl -> string != NULL; 
        jEl = jEl -> next)
    {
        if (jEl -> type & cJSON_Number
            && strcmp(jEl -> string, "id") == 0)
        {
            id = jEl -> valuedouble;
            rc = true;
        }
        else if (jEl -> type & cJSON_String
            && strcmp(jEl -> string, "first_name") == 0)
        {
            first_name = jEl -> valuestring;
        }
        else if (jEl -> type & cJSON_String
            && strcmp(jEl -> string, "last_name") == 0)
        {
            last_name = jEl -> valuestring;
        }
        else if (jEl -> type & cJSON_String
            && strcmp(jEl -> string, "username") == 0)
        {
            username = jEl -> valuestring;
        }
        else if (jEl -> type & cJSON_String
            && strcmp(jEl -> string, "language_code") == 0)
        {
            language_code = jEl -> valuestring;
        }
    }

    return rc;
}


bool TgChat ::  populatefromJson (cJSON *jChat)
{
    bool rc = false;

    for (cJSON * jEl = jChat -> child; 
        jEl != NULL && jEl -> string != NULL; 
        jEl = jEl -> next)
    {
        if (jEl -> type & cJSON_Number
            && strcmp(jEl -> string, "id") == 0)
        {
            id = jEl -> valuedouble;
            rc = true;
        }
    }

    return rc;
}
