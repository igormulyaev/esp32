#ifndef TEST_BLE_HPP
#define TEST_BLE_HPP

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace conBle {

    constexpr size_t BLE_DATA_SIZE = 64;

    union BleData 
    {
        struct {
            uint8_t length;
            char data[BLE_DATA_SIZE - sizeof(length)];
        } str;
        
        enum BleNotification {
            Subscribe,
            Unsubscribe
        };
        struct {
            uint8_t zero;
            BleNotification value;
        } notify;
    };

    extern QueueHandle_t receiveBleQueue;

    void startBle(const char *name);
    esp_err_t sendBle (const char *str, size_t len);

}

#endif // TEST_BLE_HPP