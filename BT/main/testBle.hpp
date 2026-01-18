#ifndef TEST_BLE_HPP
#define TEST_BLE_HPP

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

constexpr size_t BLE_DATA_SIZE = 64;

union BleData 
{
    struct BleString {
        uint16_t length;
        char data[BLE_DATA_SIZE - sizeof(length)];
    } str;
    
    struct BleCommand {
        uint16_t zero;
        enum BleCommandValue {
            cmdSubscribe,
            cmdUnsubscribe
        } command;
    } cmd;
};

extern QueueHandle_t receiveBleQueue;

void testBle();
esp_err_t sendBle (const char *str, size_t len);

#endif // TEST_BLE_HPP