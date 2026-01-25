#include <string>
#include <string_view>

#include "esp_check.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"

#include "consoleBle.hpp"
#include "Mhz19Uart.hpp"

static const char * const tag = "main";

// ===============================================================================
class Mhz19Uart_1_17_16 : public Mhz19Uart <Mhz19Uart_1_17_16>
{
    public:
        static constexpr uart_port_t uartNum = UART_NUM_1;
        static constexpr gpio_num_t txPin = GPIO_NUM_17;
        static constexpr gpio_num_t rxPin = GPIO_NUM_16;
};

Mhz19Uart_1_17_16 mhz19uart;

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
esp_err_t readAndSendCo2 () 
{
    int co2 = 0;
    int temperature = 0;
    const char * err = nullptr;

    esp_err_t rc = mhz19uart.readCo2 (co2, temperature, err, true);
    if (rc != ESP_OK) {
        return conBle::sendBle (err, strlen(err));
    }

    char buf[64];
    int len = snprintf (buf, sizeof(buf), "%s\r\n", err);
    
    rc = conBle::sendBle (buf, len);
    if (rc != ESP_OK) {
        return rc;
    }

    len = snprintf (buf, sizeof(buf), "CO2: %d ppm, Temp: %d C\r\n", co2, temperature);
    return conBle::sendBle (buf, len);
}

// ===============================================================================
esp_err_t processCommand (const std::string_view &cmd) 
{
    ESP_LOGI (tag, "Command received: %.*s", (int)cmd.size(), cmd.data());

    return readAndSendCo2();
}

// ===============================================================================
void mainLoop () 
{
    conBle::BleData rxBuf;
    std::string receivedData;
    TickType_t timeToWait = portMAX_DELAY;

    esp_err_t rc = ESP_OK;

    while (rc == ESP_OK) {
        BaseType_t q = xQueueReceive(conBle::receiveBleQueue, &rxBuf, timeToWait);
        if (q == pdTRUE) {
            if (rxBuf.str.length != 0) {
                size_t startFindPos = receivedData.size();
                if (startFindPos != 0) {
                    --startFindPos;
                }
                receivedData.append(rxBuf.str.data, rxBuf.str.length);

                do {
                    size_t endPos = receivedData.find("\r\n", startFindPos, 2);
                    if (endPos == std::string::npos) {
                        break;
                    }
                    std::string_view line (&receivedData[0], endPos);
                    rc = processCommand (line);

                    receivedData.erase(0, endPos + 2);
                    startFindPos = 0;
                    //rc = readAndSendCo2();
                } while (true);
            }
            else {
                switch (rxBuf.notify.value) {
                case conBle::BleData::BleNotification::Subscribe:
                    ESP_LOGI (tag, "Received command: Subscribe");
                    rc = conBle::sendBle ("Hello\r\n", 7);
                    if (rc != ESP_OK) {
                        break;
                    }
                    //rc = readAndSendCo2();
                    //timeToWait = 30000 / portTICK_PERIOD_MS;
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
            //readAndSendCo2();
        }
    } 

    ESP_LOGE (tag, "sendBle failed, rc = %d, stopping app_main", rc);

}
// ===============================================================================
extern "C"
void app_main () 
{
    ESP_LOGI (tag, "starting app_main");

    esp_reset_reason_t resetReason = esp_reset_reason();

    if (resetReason == ESP_RST_POWERON || resetReason == ESP_RST_SW) {
        ESP_RETURN_VOID_ON_ERROR(nvsInit(), tag, "nvs flash init failed");

        ESP_LOGI (tag, "NVS initialized, starting BLE test");
        
        conBle::receiveBleQueue = xQueueCreate (16, sizeof(conBle::BleData));

        conBle::startBle("CO2 Sensor");

        if (mhz19uart.init() == ESP_OK) {
            mainLoop();
        }
        mhz19uart.deinit();
    }
    else {
        ESP_LOGI (tag, "Stop processing due to reset reason %d", resetReason);
    }

    ESP_LOGI (tag, "Finish");

}
