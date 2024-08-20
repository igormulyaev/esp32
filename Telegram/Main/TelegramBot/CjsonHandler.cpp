#include "CjsonHandler.hpp"
#include "cJSON.h"

// -----------------------------------------------------------------------
CjsonHandler :: CjsonHandler (const std :: string & s)
{
    root = cJSON_Parse (s.c_str());
}

// -----------------------------------------------------------------------
CjsonHandler :: ~CjsonHandler()
{
    cJSON_Delete (root);
}