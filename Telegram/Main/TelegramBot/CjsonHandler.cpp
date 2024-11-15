#include "CjsonHandler.hpp"
#include "cJSON.h"

// -----------------------------------------------------------------------
CjsonHandler :: CjsonHandler (const char *s)
{
    root = cJSON_Parse (s);
}

// -----------------------------------------------------------------------
CjsonHandler :: ~CjsonHandler()
{
    cJSON_Delete (root);
}