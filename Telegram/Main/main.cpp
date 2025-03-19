#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "StaticObjects.hpp"
#include "TelegramTask.hpp"
#include "System/NVS.hpp"
#include "Console/Console.hpp"

static const char * const TAG = "main";

// ----------------------------------------------------------------------------------
/*
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
            "/_*", <--- without underscore!!!
            HTTP_GET
        );

        sta.WaitForDisconnect();
        ESP_LOGI(TAG, "Station disconnected");
        httpServ.Stop();
    };
}
*/
// ----------------------------------------------------------------------------------
extern "C" void app_main(void)
{
    ESP_LOGI (TAG, "Begin app_main");

    nvs.Init();

    //tgTask.createTask("TelegramTask");
    console.Run();
 
    ESP_LOGI (TAG, "End app_main");
}