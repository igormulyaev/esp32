#ifndef ANSWERPARSERUPDATE_HPP
#define ANSWERPARSERUPDATE_HPP

#include "AnswerParserBase.hpp"

class AnswerParserUpdate : public AnswerParserBase
{
    public:
        AnswerParserUpdate(const std :: string & s);
        virtual ~AnswerParserUpdate() {};

    private:
        int lastUpdateId;
};

#endif // ANSWERPARSERUPDATE_HPP