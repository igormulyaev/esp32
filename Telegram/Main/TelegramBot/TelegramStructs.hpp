#ifndef TELEGRAMSTRUCTS_HPP
#define TELEGRAMSTRUCTS_HPP

#include <cstdint>
#include <string>

struct cJSON;

struct TgUser
{
    uint64_t id = 0;
    bool is_bot = false;
    std::string first_name;
    std::string last_name;
    std::string username;
    char language_code[3] = {0};

    bool populatefromJson (cJSON * jUser);
};

struct TgChat
{
    uint64_t id = 0;

    bool populatefromJson (cJSON * jChat);
};

struct TgMessage
{
    uint64_t message_id = 0;
    TgUser from;
    TgChat chat;
    uint64_t date = 0;
    std::string text;

    bool populatefromJson (cJSON * jMessage);
};
#endif // TELEGRAMSTRUCTS_HPP