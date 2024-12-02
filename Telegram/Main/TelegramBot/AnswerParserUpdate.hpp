#ifndef ANSWERPARSERUPDATE_HPP
#define ANSWERPARSERUPDATE_HPP

#include "AnswerParserBase.hpp"

class AnswerParserUpdate : public AnswerParserBase
{
    public:
        AnswerParserUpdate(const std :: string & s);
        virtual ~AnswerParserUpdate() {};

        uint32_t getLastUpdateId() { return lastUpdateId; }

    private:
        uint32_t lastUpdateId;
};

#endif // ANSWERPARSERUPDATE_HPP