#include "AnswerParserUpdate.hpp"
#include "cJSON.h"
#include <cstring>

// -----------------------------------------------------------------------
AnswerParserUpdate :: AnswerParserUpdate (const std::string & s)
    : AnswerParserBase (s)
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
                    message.populatefromJson (jREl);
                }
            }
            
            if (jResults -> next != NULL)
            {
                // Not a single message in the Result?
            }
        }
    }
}
