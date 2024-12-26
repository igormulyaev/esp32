#ifndef ANSWERPARSERSEND_HPP
#define ANSWERPARSERSEND_HPP

#include "AnswerParserBase.hpp"

class AnswerParserSend : public AnswerParserBase
{
    public:
        AnswerParserSend(const std::string & s);
        virtual ~AnswerParserSend() {};
};

#endif // ANSWERPARSERSEND_HPP