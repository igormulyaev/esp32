#ifndef TELEGRAMBOT_HPP
#define TELEGRAMBOT_HPP

#include <string>
#include "HttpsClient/HttpsClient.hpp"

class TelegramBot 
{
    public:
        TelegramBot();
        ~TelegramBot() {}

        void Init (const char *id, const char *key);
        void Process();

    private:
        // State machine
        enum States
        {
            tgStart = 0
            , tgGetMe
            , tgUpdate
            , tgStop
            , tgExit
        };

        TelegramBot :: States Start();
        TelegramBot :: States GetMe();
        TelegramBot :: States Update();
        TelegramBot :: States Stop();

        TelegramBot :: States state;

        // bot params
        std :: string Firstname;
        std :: string Username;

        // bot workdata
        HttpsClient client;
        std :: string basicUrl;
        
        std :: string fullUrl;

        static const char * const TAG;
};

#endif