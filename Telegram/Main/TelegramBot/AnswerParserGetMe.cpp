#include "AnswerParserGetMe.hpp"
#include "cJSON.h"
#include <cstring>

// -----------------------------------------------------------------------
AnswerParserGetMe :: AnswerParserGetMe (const std :: string & s)
    : AnswerParserBase (s)
{}

// -----------------------------------------------------------------------
void AnswerParserGetMe :: Parse()
{
    AnswerParserBase :: Parse();

    if (isOk)
    {
        for (cJSON * jEl = result -> child; jEl != NULL; jEl = jEl -> next)
        {
            if ((jEl -> type & cJSON_String) != 0)
            {
                if (strcmp (jEl -> string, "first_name") == 0)
                {
                    firstname = jEl -> valuestring;
                }
                else if (strcmp (jEl -> string, "username") == 0)
                {
                    username = jEl -> valuestring;
                }
            }
        }
    }
}
