#ifndef ANSWERPARSERBASE_HPP
#define ANSWERPARSERBASE_HPP

#include "CjsonHandler.hpp"

class AnswerParserBase
{
    public:
        AnswerParserBase (const std :: string & s);
        virtual ~AnswerParserBase() {};

        virtual void Parse();

        bool getIsOk() { return isOk; }

    protected:
        CjsonHandler json;
        bool isOk;
        const cJSON * result;
};

#endif // ANSWERPARSERBASE_HPP