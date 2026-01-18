#ifndef TEST_BLE_HPP
#define TEST_BLE_HPP

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

struct bleData 
{
    size_t length;
    char data[64 - sizeof(size_t)];
};

extern QueueHandle_t receiveBleQueue;

void testBle();
esp_err_t sendBle (const char *str, size_t len);

#endif // TEST_BLE_HPP