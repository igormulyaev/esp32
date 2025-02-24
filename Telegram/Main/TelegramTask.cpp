#include "TelegramTask.hpp"
#include "TelegramBot/TelegramBot.hpp"
#include "WiFiStation/WiFiStation.hpp"
#include "WiFiStation/NVS.hpp"
#include "StaticObjects.hpp"

#include "esp_log.h"

// -----------------------------------------------------------------------
void TelegramTask :: execute()
{
    ESP_LOGI (TAG, "Begin execute");

    nvs.Init();

    sta.Init();

    ESP_LOGI (TAG, "End Init");

    sta.Run();

    TelegramBot bot;

    while (true)
    {
        sta.WaitForConnect();
        ESP_LOGI (TAG, "Station connected");

        bot.Init (tgBotId, tgBotKey);

        bot.Process();

        sta.WaitForDisconnect();
        ESP_LOGI (TAG, "Station disconnected");
    }
}

// -----------------------------------------------------------------------
const char * const TelegramTask :: TAG = "TelegramTask";