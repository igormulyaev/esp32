#include "AnswerParserBase.hpp"
#include "cJSON.h"
#include <cstring>

// -----------------------------------------------------------------------
AnswerParserBase :: AnswerParserBase (const std::string & s)
    : json (s.c_str())
{
    const cJSON * jRoot = json.getRoot();

    if (jRoot != NULL)
    {
        const cJSON * jOk = jRoot -> child;
        
        isOk = jOk != NULL
            && strcmp (jOk -> string, "ok") == 0
            && (jOk -> type & cJSON_True) != 0
            && jOk -> next != NULL;

        if (isOk)
        {
            result = jOk -> next;
        }
    }
}
