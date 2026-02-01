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

enum BleEvents {
    Event_ReceiveBle,
    Event_Timeout,
    Event_Subscribe,
    Event_Unsubscribe,
    Event_None
};

static std::string receivedData;

static TickType_t sensorScanNext = portMAX_DELAY;
static TickType_t sensorScanPeriod = 0; // Disabled by default

static TickType_t consoleSendNext = portMAX_DELAY;
static TickType_t consoleSendPeriod = 0; // Disabled by default

static bool isDebug = false;

BleEvents bleProcess ();

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
esp_err_t dryRead () 
{
    const char * res = nullptr;
    mhz19uart.dryRead (res);
    return sendErr (res);
}

// ===============================================================================
esp_err_t onCommand (const std::string_view &cmd) 
{
    ESP_LOGI (tag, "Command: %.*s", (int)cmd.size(), cmd.data());

    // Disable periodic sending by default
    consoleSendPeriod = 0; 
    consoleSendNext = portMAX_DELAY;

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
            unsigned int readInterval = 0;
            std::from_chars_result r = std::from_chars (durationStr.data(), durationStr.data() + durationStr.size(), readInterval);
            if (r.ec == std::errc{} && readInterval > 0) {
                char buf[64];
                int len = sprintf (buf, "Set read interval to %u s\r\n", readInterval);
                esp_err_t rc = conBle::sendBle (buf, len);
                if (rc != ESP_OK) {
                    return rc;
                }
                consoleSendPeriod = readInterval * 1000 / portTICK_PERIOD_MS;
                consoleSendNext = xTaskGetTickCount() + consoleSendPeriod;
            }
        }
        return readAndSendCo2 ();
    }
    else if (cmd == "dryread") {
        return dryRead ();
    }
    else if (cmd == "calibrate") {
        return calibrateZero ();
    }
    else if (cmd == "alarm") {
        return readAlarm ();
    }
    else if (cmd == "help") {
        const char * helpStr =
            "read [n]\r\n"
            "dryread\r\n"
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
    return conBle::sendBle ("Hello\r\n", 7);
}

// ===============================================================================
esp_err_t onUnsubscribe () 
{
    consoleSendNext = portMAX_DELAY;
    return ESP_OK;
}

// ===============================================================================
esp_err_t sensorScan () 
{
    ESP_LOGI (tag, "sensorScan called");
    return ESP_OK;
}

// ===============================================================================
esp_err_t consoleSend () 
{
    return readAndSendCo2 ();
}

// ===============================================================================
void mainLoop ()
{
    esp_err_t rc = ESP_OK;

    while (rc == ESP_OK) {
        size_t oldReceivedSize = receivedData.size();

        BleEvents event = bleProcess ();

        switch (event) {
        case Event_ReceiveBle: {
            size_t startFindPos = oldReceivedSize == 0 ? 0 : oldReceivedSize - 1;
            
            while (true) {
                size_t endPos = receivedData.find("\r\n", startFindPos, 2);
                if (endPos == std::string::npos) {
                    break;
                }
                std::string_view line (&receivedData[0], endPos);
                rc = onCommand (line);

                receivedData.erase(0, endPos + 2);
                startFindPos = 0;
            } 
            break;
        }
        case Event_Subscribe:
            rc = onSubscribe ();
            break;

        case Event_Unsubscribe:
            rc = onUnsubscribe ();
            break;

        case Event_Timeout: {
            TickType_t now = xTaskGetTickCount();
            if (now >= sensorScanNext) {
                if (sensorScanPeriod == 0) {
                    // Disabled
                    sensorScanNext = portMAX_DELAY;
                }
                else {
                    sensorScanNext += sensorScanPeriod;
                }
                rc = sensorScan ();
            }
            if (now >= consoleSendNext) {
                if (consoleSendPeriod == 0) {
                    // Disabled
                    consoleSendNext = portMAX_DELAY;
                }
                else {
                    consoleSendNext += consoleSendPeriod;
                }
                rc = consoleSend ();
            }
            break;
        }
        case Event_None:
            rc = ESP_FAIL; // Something wrong happened
            break;
        }
    }
    ESP_LOGE (tag, "mainLoop failed, rc = %d, stopping", rc);
}

// ===============================================================================
TickType_t getTimeToNextEvent()
{
    TickType_t now = xTaskGetTickCount();
    TickType_t timeToSensor = (sensorScanNext == portMAX_DELAY)
        ? portMAX_DELAY
        : (sensorScanNext > now) 
            ? (sensorScanNext - now)
            : 0;
    TickType_t timeToConsole = (consoleSendNext == portMAX_DELAY)
        ? portMAX_DELAY
        : (consoleSendNext > now)
            ? (consoleSendNext - now)
            : 0;

    return (timeToSensor < timeToConsole) ? timeToSensor : timeToConsole;
}

// ===============================================================================
BleEvents bleProcess () 
{
    conBle::BleData rxBuf;
    
    TickType_t timeToWait = getTimeToNextEvent();
    
    if (xQueueReceive(conBle::receiveBleQueue, &rxBuf, timeToWait) == pdTRUE) {
        if (rxBuf.str.length != 0) {
            receivedData.append(rxBuf.str.data, rxBuf.str.length);
            return Event_ReceiveBle;
        }
        else {
            switch (rxBuf.notify.value) {
            case conBle::BleData::BleNotification::Subscribe:
                ESP_LOGI (tag, "BLE Subscribe");
                return Event_Subscribe;

            case conBle::BleData::BleNotification::Unsubscribe:
                ESP_LOGI (tag, "BLE Unsubscribe");
                return Event_Unsubscribe;

            default:
                ESP_LOGW (tag, "Unknown BLE command: %d", rxBuf.notify.value);
                return Event_None;
            }
        }
    }
    else {
        return Event_Timeout;
    }
}

// ===============================================================================
extern "C"
void app_main () 
{
    ESP_LOGI (tag, "starting app_main");

    esp_reset_reason_t resetReason = esp_reset_reason();

    if (resetReason == ESP_RST_POWERON || resetReason == ESP_RST_SW) {
        ESP_RETURN_VOID_ON_ERROR(nvsInit(), tag, "nvs flash init failed");

        const char * err = nullptr;
        ESP_RETURN_VOID_ON_ERROR(mhz19uart.init(err), tag, "mhz19uart init failed: %s", err);

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
