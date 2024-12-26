#ifndef ANSWERPARSERUPDATE_HPP
#define ANSWERPARSERUPDATE_HPP

#include "AnswerParserBase.hpp"
#include "TelegramStructs.hpp"

class AnswerParserUpdate : public AnswerParserBase
{
    public:
        AnswerParserUpdate(const std::string & s);
        virtual ~AnswerParserUpdate() {};

        uint32_t getUpdateId() const { return update_id; }
        TgMessage const & getMessage() const { return message; }

    private:
        uint32_t update_id = 0;
        TgMessage message;
};

#endif // ANSWERPARSERUPDATE_HPP