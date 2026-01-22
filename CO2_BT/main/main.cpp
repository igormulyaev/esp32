#include "consoleBle.hpp"

#include "esp_check.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"

static const char * const tag = "main";

// ===============================================================================
static esp_err_t nvsInit () 
{
    esp_err_t rc = nvs_flash_init();

    if (rc == ESP_ERR_NVS_NO_FREE_PAGES || rc == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_RETURN_ON_ERROR(nvs_flash_erase(), tag, "erasing nvs flash failed");
        rc = nvs_flash_init();
    }
    
    return rc;
}

// ===============================================================================
extern "C"
void app_main () 
{
    ESP_LOGI (tag, "starting app_main");

    ESP_RETURN_VOID_ON_ERROR(nvsInit(), tag, "nvs flash init failed");

    ESP_LOGI (tag, "NVS initialized, starting BLE test");
    
    conBle::receiveBleQueue = xQueueCreate (16, sizeof(conBle::BleData));

    conBle::startBle("CO2 Sensor");

    conBle::BleData rxBuf;

    esp_err_t rc = ESP_OK;
    int n = 0;

    TickType_t timeToWait = portMAX_DELAY;
    do {
        BaseType_t q = xQueueReceive(conBle::receiveBleQueue, &rxBuf, timeToWait);
        if (q == pdTRUE) {
            if (rxBuf.str.length != 0) {
                const char * src = rxBuf.str.data;
                char sBuf[80];
                char * dst = sBuf;
                for (size_t i = rxBuf.str.length; i != 0; --i, ++src) {
                    switch (*src) {
                    case '\r':
                        *dst++ = 0xe2;
                        *dst++ = 0x90;
                        *dst++ = 0x8d;
                        break;
                    case '\n':
                        *dst++ = 0xe2;
                        *dst++ = 0x90;
                        *dst++ = 0x8a;
                        break;
                    default:
                        *dst++ = *src;
                    }
                }
                *dst = '\0';
                ESP_LOGI (tag, "Received data (%d bytes): %s", rxBuf.str.length, sBuf);
            }
            else {
                switch (rxBuf.notify.value) {
                case conBle::BleData::BleNotification::Subscribe:
                    ESP_LOGI (tag, "Received command: Subscribe");
                    rc = conBle::sendBle ("Hello\r\n", 7);
                    timeToWait = 10000 / portTICK_PERIOD_MS;
                    break;

                case conBle::BleData::BleNotification::Unsubscribe:
                    ESP_LOGI (tag, "Received command: Unsubscribe");
                    timeToWait = portMAX_DELAY;
                    break;

                default:
                    ESP_LOGW (tag, "Received unknown command: %d", rxBuf.notify.value);
                    break;
                }
            }
        }
        else {
            char txBuf[32];
            int len = sprintf (txBuf, "RSVP %d\r\n", n);
            rc = conBle::sendBle (txBuf, len);
            ++n;
        }
    } while (rc == ESP_OK);

    ESP_LOGE (tag, "sendBle failed, rc = %d, stopping app_main", rc);
}
