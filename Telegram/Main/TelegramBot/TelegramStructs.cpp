#include "TelegramStructs.hpp"
#include "cJSON.h"
#include <cstring>
//#include <cstdlib>


bool TgUser :: populatefromJson(cJSON * jUser)
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
            strncpy (language_code, jEl -> valuestring, 2);
            language_code[2] = 0;
        }
    }

    return rc;
}


bool TgChat ::  populatefromJson (cJSON * jChat)
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

bool TgMessage :: populatefromJson (cJSON * jMessage)
{
    bool rc = false;

    for (cJSON * jEl = jMessage -> child; 
        jEl != NULL && jEl -> string != NULL; 
        jEl = jEl -> next)
    {
        if (jEl -> type & cJSON_Number
            && strcmp(jEl -> string, "message_id") == 0)
        {
            message_id = jEl -> valuedouble;
            rc = true;
        }
        else if (jEl -> type & cJSON_Number
            && strcmp(jEl -> string, "date") == 0)
        {
            date = jEl -> valuedouble;
        }
        else if (jEl -> type & cJSON_String
            && strcmp(jEl -> string, "text") == 0)
        {
            text = jEl -> valuestring;
        }
        else if (jEl -> type & cJSON_Object
            && strcmp(jEl -> string, "from") == 0)
        {
            rc = from.populatefromJson(jEl);
        }
        else if (jEl -> type & cJSON_Object
            && strcmp(jEl -> string, "chat") == 0)
        {
            rc = chat.populatefromJson(jEl);
        }
    }

    return rc;
}