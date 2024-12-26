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
            , tgReadOldMessages
            , tgReadMessage
            , tgSendMessage
            , tgStop
            , tgExit
        };

        TelegramBot :: States Start();
        TelegramBot :: States GetMe();
        TelegramBot :: States Update();
        TelegramBot :: States Send();
        TelegramBot :: States Stop();

        TelegramBot :: States state;

        esp_err_t PerformGet (const std::string & url);
        esp_err_t PerformPost (const std::string & url, const std::string & data, const char * contentType = "application/json");

        // bot params
        std::string Firstname;
        std::string Username;

        // message received

        // bot workdata
        HttpsClient client;
        std::string basicUrl;
        uint32_t lastMsgId;
        
        static const char * const TAG;
};

#endif