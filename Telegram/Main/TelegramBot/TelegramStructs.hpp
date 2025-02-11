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
    void clear() { id = 0; is_bot = false; first_name.clear(); last_name.clear(); username.clear(); language_code[0] = 0; }
};

struct TgChat
{
    uint64_t id = 0;

    bool populatefromJson (cJSON * jChat);
    void clear() { id = 0; }
};

struct TgMessage
{
    uint64_t message_id = 0;
    TgUser from;
    TgChat chat;
    uint64_t date = 0;
    std::string text;

    bool populatefromJson (cJSON * jMessage);
    void clear() { message_id = 0; from.clear(); chat.clear(); date = 0; text.clear(); }
};
#endif // TELEGRAMSTRUCTS_HPP