#include "AnswerParserUpdate.hpp"
#include "cJSON.h"
#include <cstring>

// -----------------------------------------------------------------------
AnswerParserUpdate :: AnswerParserUpdate (const std :: string & s)
    : AnswerParserBase (s), lastUpdateId(0)
{
    if (isOk)
    {
        // Process the message (it should be only one)
        cJSON * jMsg = result -> child;

        if (jMsg != NULL)
        {
            // Loop through message elements
            for (cJSON * jMEl = jMsg -> child; jMEl != NULL; jMEl = jMEl -> next)
            {
                if (jMEl -> string != NULL)
                {
                    if (jMEl -> type & cJSON_Number 
                        && strcmp (jMEl -> string, "update_id") == 0)
                    {
                        lastUpdateId = jMEl -> valueint;
                    }
                    else if (jMEl -> type & cJSON_Object
                        && strcmp (jMEl -> string, "message") == 0)
                    {
                        // message
                        printf ("Message content\n");
                    }
                }
            }
            
            if (jMsg -> next != NULL)
            {
                // Not single message?
            }
        }
    }
}
