#ifndef TELEGRAMSTRUCTS_HPP
#define TELEGRAMSTRUCTS_HPP

#include <cstdint>
#include <string>

struct cJSON;

struct TgUser
{
    uint64_t id;
    bool is_bot;
    std::string first_name;
    std::string last_name;
    std::string username;
    std::string language_code;

    TgUser(): id(0), is_bot(false)
    {}

    bool populatefromJson (cJSON *jUser);
};

struct TgChat
{
    uint64_t id;

    TgChat(): id(0)
    {}
    bool populatefromJson (cJSON *jChat);
};


#endif // TELEGRAMSTRUCTS_HPP