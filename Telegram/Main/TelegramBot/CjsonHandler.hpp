#ifndef CJSONHANDLER_HPP
#define CJSONHANDLER_HPP

#include <string>

struct cJSON;

class CjsonHandler
{
    public:
        CjsonHandler (const char *s);
        ~CjsonHandler();
    
        const cJSON * getRoot() { return root; }

    private:
        cJSON * root;
};

#endif // CJSONHANDLER_HPP