#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "StaticObjects.hpp"
#include "TelegramTask.hpp"

#include "esp_log.h"

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
    tgTask.createTask("TelegramTask");
    ESP_LOGI (TAG, "End app_main");
}