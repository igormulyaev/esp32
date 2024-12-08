#include "AnswerParserUpdate.hpp"
#include "cJSON.h"
#include <cstring>

// -----------------------------------------------------------------------
AnswerParserUpdate :: AnswerParserUpdate (const std :: string & s)
    : AnswerParserBase (s), update_id(0), message_id(0), from_id(0), date(0)
{
    if (isOk)
    {
        // Process the message (it should be only one)
        cJSON * jResults = result -> child;

        if (jResults != NULL)
        {
            // Loop through result elements
            for (cJSON * jREl = jResults -> child; 
                jREl != NULL && jREl -> string != NULL; 
                jREl = jREl -> next)
            {
                if (jREl -> type & cJSON_Number 
                    && strcmp (jREl -> string, "update_id") == 0)
                {
                    update_id = jREl -> valueint;
                }
                else if (jREl -> type & cJSON_Object
                    && strcmp (jREl -> string, "message") == 0)
                {
                    for (cJSON *jMsg = jREl -> child; 
                        jMsg != NULL && jMsg -> string != NULL; 
                        jMsg = jMsg -> next)
                    {
                        if (jMsg -> type & cJSON_Number
                            && strcmp (jMsg -> string, "message_id") == 0)
                        {
                            message_id = jMsg -> valueint;
                        }
                        else if (jMsg -> type & cJSON_Object
                            && strcmp (jMsg -> string, "from") == 0)
                        {
                            for (cJSON * jFrom = jMsg -> child; 
                                jFrom != NULL && jFrom -> string != NULL; 
                                jFrom = jFrom -> next)
                            {
                                if (jFrom -> type & cJSON_Number
                                    && strcmp(jFrom -> string, "id") == 0)
                                {
                                    from_id = jFrom ->valueint;
                                }
                                else if (jFrom -> type & cJSON_String
                                    && strcmp(jFrom -> string, "first_name") == 0)
                                {
                                    from_first_name = jFrom -> valuestring;
                                }
                            }
                        }
                        else if (jMsg -> type & cJSON_Number
                            && strcmp (jMsg -> string, "date") == 0)
                        {
                            date = jMsg -> valueint;
                        }
                        else if (jMsg -> type & cJSON_String
                            && strcmp (jMsg -> string, "text") == 0)
                        {
                            text = jMsg -> valuestring;
                        }
                    }
                }
            }
            
            if (jResults -> next != NULL)
            {
                // Not a single message in the Result?
            }
        }
    }
}

/*
{
    "ok":true
    ,"result":[
        {
            "update_id":975830317
            ,"message":{
                "message_id":11
                ,"from":{
                    "id":584216360
                    ,"is_bot":false
                    ,"first_name":"I"
                    ,"language_code":"ru"
                }
                ,"chat":{
                    "id":584216360
                    ,"first_name":"I"
                    ,"type":"private"
                }
                ,"date":1733672804
                ,"text":"Test"
            }
        }
    ]
}

*/