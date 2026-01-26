#include <string>
#include <string_view>
#include <charconv>

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

static TickType_t timeToWait = portMAX_DELAY;
bool isDebug = false;

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
esp_err_t sendErr (const char * err) 
{
    char buf[128];
    int len = snprintf (buf, sizeof(buf), "%s\r\n", err);
    return conBle::sendBle (buf, len);
}
// ===============================================================================
esp_err_t readAndSendCo2 () 
{
    int co2 = 0;
    int temperature = 0;
    const char * err = nullptr;

    esp_err_t rc = mhz19uart.readCo2 (co2, temperature, err, isDebug);
    if (rc != ESP_OK) {
        return sendErr (err);
    }

    if (isDebug) {
        rc = sendErr (err);
        if (rc != ESP_OK) {
            return rc;
        }
    }

    char buf[64];
    int len = snprintf (buf, sizeof(buf), "CO2: %d ppm, Temp: %d C\r\n", co2, temperature);
    return conBle::sendBle (buf, len);
}

// ===============================================================================
esp_err_t calibrateZero () 
{
    const char * res = nullptr;
    mhz19uart.calibrateZero (res);
    return sendErr (res);
}

// ===============================================================================
esp_err_t readAlarm () 
{
    const char * res = nullptr;
    mhz19uart.readAlarm (res);
    return sendErr (res);
}

// ===============================================================================
esp_err_t onCommand (const std::string_view &cmd) 
{
    ESP_LOGI (tag, "Command: %.*s", (int)cmd.size(), cmd.data());

    timeToWait = portMAX_DELAY;
    
    if (cmd == "debug on") {
        isDebug = true;
        return conBle::sendBle ("Debug ON\r\n", 10);
    }
    else if (cmd == "debug off") {
        isDebug = false;
        return conBle::sendBle ("Debug OFF\r\n", 11);
    }
    else if (cmd.starts_with("read")) {
        if (cmd.size() > 5 && cmd[4] == ' ') {
            std::string_view durationStr = cmd.substr(5);
            unsigned int newTimeout;
            std::from_chars_result r = std::from_chars (durationStr.data(), durationStr.data() + durationStr.size(), newTimeout);
            if (r.ec == std::errc{} && newTimeout > 0) {
                char buf[64];
                int len = sprintf (buf, "Set read interval to %u s\r\n", newTimeout);
                esp_err_t rc = conBle::sendBle (buf, len);
                if (rc != ESP_OK) {
                    return rc;
                }
                timeToWait = newTimeout * 1000 / portTICK_PERIOD_MS;
            }
        }
        return readAndSendCo2 ();
    }
    else if (cmd == "init") {
        const char * err = nullptr;
        esp_err_t rc = mhz19uart.init (err);
        if (rc != ESP_OK) {
            return sendErr (err);
        }
        else {
            return conBle::sendBle ("Initialized\r\n", 13);
        }
    }
    else if (cmd == "deinit") {
        const char * err = nullptr;
        esp_err_t rc = mhz19uart.deinit (err);
        if (rc != ESP_OK) {
            return sendErr (err);
        }
        else {
            return conBle::sendBle ("Deinitialized\r\n", 15);
        }
    }
    else if (cmd == "calibrate") {
        return calibrateZero ();
    }
    else if (cmd == "alarm") {
        return readAlarm ();
    }
    else if (cmd == "help") {
        const char * helpStr =
            "init\r\n"
            "deinit\r\n"
            "read [n]\r\n"
            "debug on|off\r\n"
            "calibrate\r\n"
            "alarm\r\n";
        return conBle::sendBle (helpStr, strlen(helpStr));
    }
    else {
        return conBle::sendBle ("Unknown command\r\n", 17);
    }
}

// ===============================================================================
esp_err_t onSubscribe () 
{
    timeToWait = portMAX_DELAY;
    return conBle::sendBle ("Hello\r\n", 7);
}

// ===============================================================================
esp_err_t onUnsubscribe () 
{
    timeToWait = portMAX_DELAY;
    return ESP_OK;
}

// ===============================================================================
esp_err_t onTimeout () 
{
    return readAndSendCo2 ();
}

// ===============================================================================
void mainLoop () 
{
    conBle::BleData rxBuf;
    std::string receivedData;

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
                    rc = onCommand (line);

                    receivedData.erase(0, endPos + 2);
                    startFindPos = 0;
                } while (true);
            }
            else {
                switch (rxBuf.notify.value) {
                case conBle::BleData::BleNotification::Subscribe:
                    ESP_LOGI (tag, "BLE Subscribe");
                    rc = onSubscribe ();
                    break;

                case conBle::BleData::BleNotification::Unsubscribe:
                    ESP_LOGI (tag, "BLE Unsubscribe");
                    rc = onUnsubscribe ();
                    break;

                default:
                    ESP_LOGW (tag, "Unknown BLE command: %d", rxBuf.notify.value);
                    break;
                }
            }
        }
        else {
            rc = onTimeout ();
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

        mainLoop();
    }
    else {
        ESP_LOGI (tag, "Stop processing due to reset reason %d", resetReason);
    }

    ESP_LOGI (tag, "Finish");

}
