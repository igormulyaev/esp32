#ifndef ANSWERPARSERUPDATE_HPP
#define ANSWERPARSERUPDATE_HPP

#include "AnswerParserBase.hpp"
#include "TelegramStructs.hpp"

class AnswerParserUpdate : public AnswerParserBase
{
    public:
        AnswerParserUpdate(const std::string & s, TgMessage &msg);
        virtual ~AnswerParserUpdate() {};

        uint32_t getUpdateId() const { return update_id; }

    private:
        uint32_t update_id = 0;
};

#endif // ANSWERPARSERUPDATE_HPP