#include "AnswerParserUpdate.hpp"
#include "cJSON.h"
#include <cstring>

// -----------------------------------------------------------------------
AnswerParserUpdate :: AnswerParserUpdate (const std :: string & s)
    : AnswerParserBase (s), lastUpdateId(0)
{
    if (isOk)
    {
        // Loop through messages
        for (cJSON * jMsg = result -> child; jMsg != NULL; jMsg = jMsg -> next)
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
                        printf ("New update_id = %d\n", lastUpdateId);
                    }
                    else if (jMEl -> type & cJSON_Object
                        && strcmp (jMEl -> string, "message") == 0)
                    {
                        // message
                        printf ("Message content\n");
                    }
                }
            }
        }
    }
}
