#ifndef ANSWERPARSERGETME_HPP
#define ANSWERPARSERGETME_HPP

#include "AnswerParserBase.hpp"

class AnswerParserGetMe : public AnswerParserBase
{
    public:
        AnswerParserGetMe(const std :: string & s);
        virtual ~AnswerParserGetMe() {};

        virtual void Parse();

        std :: string getUsername() { return username; }
        std :: string getFirstname() { return firstname; }

    private:
        std :: string username;
        std :: string firstname;
};

#endif // ANSWERPARSERGETME_HPP