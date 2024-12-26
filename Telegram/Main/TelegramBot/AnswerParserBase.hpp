#ifndef ANSWERPARSERBASE_HPP
#define ANSWERPARSERBASE_HPP

#include "CjsonHandler.hpp"

class AnswerParserBase
{
    public:
        AnswerParserBase (const std :: string & s);
        virtual ~AnswerParserBase() {};

        bool getIsOk() { return isOk; }

    protected:
        CjsonHandler json;
        bool isOk = false;
        const cJSON * result = NULL;
};

#endif // ANSWERPARSERBASE_HPP