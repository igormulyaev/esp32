#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "StaticObjects.hpp"
#include "WiFiStation/WiFiStation.hpp"
#include "WiFiStation/NVS.hpp"
#include "HttpServer/HttpServerBase.hpp"
#include "HttpServer/HttpUriTest.hpp"

#include "HttpsClient/HttpsClient.hpp"

#include "TelegramBot/TelegramBot.hpp"

#include "esp_log.h"

static const char * const TAG = "main";

// ----------------------------------------------------------------------------------
void TestSimpleHttpServer ()
{
    nvs.Init();

    sta.Init();

    ESP_LOGI(TAG, "End Init");

    sta.Run();

    HttpServerBase httpServ;
    HttpUriTest helloWorld;

    while (true)
    {
        sta.WaitForConnect();
        ESP_LOGI(TAG, "Station connected");
        httpServ.Start();

        httpServ.RegisterUriHandler (
            helloWorld,
            "/*",
            HTTP_GET
        );

        sta.WaitForDisconnect();
        ESP_LOGI(TAG, "Station disconnected");
        httpServ.Stop();
    };
}

// ----------------------------------------------------------------------------------
void TestHttpClient ()
{
    nvs.Init();

    sta.Init();

    ESP_LOGI(TAG, "End Init");

    sta.Run();

    HttpsClient clnt (cert_pem_start);

    while (true)
    {
        sta.WaitForConnect();
        ESP_LOGI(TAG, "Station connected");

        clnt.Init ("https://some.test.address");

        clnt.Perform();

        ESP_LOGI (TAG, "Result = \"%s\"", clnt.result.c_str());

        /*
        cJSON *root = cJSON_Parse (clnt.result.c_str());

        char *jp = cJSON_Print (root);
        ESP_LOGI (TAG, "Parsed JSON = \"%s\"", jp);
        free (jp);    

        cJSON_Delete (root);
        */
        clnt.Cleanup();

        sta.WaitForDisconnect();
        ESP_LOGI(TAG, "Station disconnected");
    }
}

// ----------------------------------------------------------------------------------
void TestTelegramBot ()
{
    ESP_LOGI(TAG, "Begin Init");

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
        ESP_LOGI(TAG, "Station disconnected");
    }
}

// ----------------------------------------------------------------------------------
extern "C" void app_main(void)
{
    TestTelegramBot();
}