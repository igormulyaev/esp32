#include "Console.hpp"
#include <cstddef>
#include <cstdio>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void Console :: Run()
{
    ESP_LOGI (TAG, "Begin Run");

    while (true)
    {
        readLine();
        ESP_LOGI (TAG, "You entered: %s", buf);
    }

    ESP_LOGI (TAG, "End Run");
}

void Console :: readLine()
{
    size_t len = 0;
    while (len < MAX_CMD_LEN)
    {
        int c = getchar();
        if (c == EOF)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }
        putchar(c);
        if (c == '\n' || c == '\r')
        {
            break;
        }
        else
        {
            buf[len++] = c;
        }
    }
    buf[len] = '\0';
}

const char * const Console :: TAG = "Console";