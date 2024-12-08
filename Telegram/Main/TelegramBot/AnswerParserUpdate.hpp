#ifndef ANSWERPARSERUPDATE_HPP
#define ANSWERPARSERUPDATE_HPP

#include "AnswerParserBase.hpp"

class AnswerParserUpdate : public AnswerParserBase
{
    public:
        AnswerParserUpdate(const std :: string & s);
        virtual ~AnswerParserUpdate() {};

        uint32_t getUpdateId() const { return update_id; }
        uint32_t getMessageId() const { return message_id; }
        uint32_t getFromId() const { return from_id; }
        std :: string const & getFromFirstName() const { return from_first_name; }
        uint32_t getDate() const { return date; }
        std :: string const & getText() const { return text; }

    private:
        uint32_t update_id;
        uint32_t message_id;
        uint32_t from_id;
        std :: string from_first_name;
        uint32_t date;
        std :: string text;
};

#endif // ANSWERPARSERUPDATE_HPP