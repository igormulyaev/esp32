#ifndef ANSWERPARSERUPDATE_HPP
#define ANSWERPARSERUPDATE_HPP

#include "AnswerParserBase.hpp"
#include "TelegramStructs.hpp"

class AnswerParserUpdate : public AnswerParserBase
{
    public:
        AnswerParserUpdate(const std :: string & s);
        virtual ~AnswerParserUpdate() {};

        uint32_t getUpdateId() const { return update_id; }
        uint32_t getMessageId() const { return message_id; }
        TgUser const & getFrom() const { return from; }
        TgChat const & getChat() const { return chat; }
        uint32_t getDate() const { return date; }
        std::string const & getText() const { return text; }

    private:
        uint32_t update_id;
        uint32_t message_id;
        TgUser from;
        TgChat chat;
        uint32_t date;
        std::string text;
};

#endif // ANSWERPARSERUPDATE_HPP