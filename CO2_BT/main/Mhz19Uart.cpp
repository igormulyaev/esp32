#include "Mhz19Uart.hpp"
#include "esp_check.h"

// ===================================================================================================
esp_err_t Mhz19UartImpl :: init(uart_port_t uartNum, gpio_num_t txPin, gpio_num_t rxPin, const char * &err)
{
    esp_err_t rc = uart_driver_install (uartNum, 129 /* minimum size, must be > 128 */, 0, 0, nullptr, 0);
    if (rc != ESP_OK) {
        err = "uart_driver_install failed";
        ESP_LOGE (tag, "%s", err);
        return rc;
    }

    uart_config_t uartConfig = {
        .baud_rate = baudRate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
        .flags = {
            .allow_pd = false,
            .backup_before_sleep = false,
        },
    };

    rc = uart_param_config (uartNum, &uartConfig);
    if (rc != ESP_OK) {
        err = "uart_param_config failed";
        ESP_LOGE (tag, "%s", err);
        return rc;
    }

    rc = uart_set_pin (uartNum, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (rc != ESP_OK) {
        err = "uart_set_pin failed";
        ESP_LOGE (tag, "%s", err);
        return rc;
    }

    err = nullptr;
    return ESP_OK;
}

// ===================================================================================================
esp_err_t Mhz19UartImpl :: deinit (uart_port_t uartNum, gpio_num_t txPin, gpio_num_t rxPin, const char * &err)
{
    esp_err_t rc = uart_driver_delete (uartNum);
    if (rc != ESP_OK) {
        err = "uart_driver_delete failed";
        ESP_LOGE (tag, "%s", err);
    }
    else {
        err = nullptr;
    }
    return rc;
}

// ===================================================================================================
esp_err_t Mhz19UartImpl :: readCo2 (int &co2, int &temperature, uart_port_t uartNum, gpio_num_t txPin, gpio_num_t rxPin, const char * &err, bool isDebug)
{
    uint8_t buf[cmdSize];
    esp_err_t rc = sendCommand (requestVal, buf, uartNum, txPin, rxPin, err, isDebug);
    if (rc != ESP_OK) {
        return rc;
    }
    co2 = (buf[2] << 8) | buf[3];

    temperature = buf[4] != 0 ? buf[4] - 40 : INT_MIN;
    
    return ESP_OK;
}

// ===================================================================================================
esp_err_t Mhz19UartImpl :: sendCommand (
    const uint8_t * command
    , uint8_t * buf
    , uart_port_t uartNum
    , gpio_num_t txPin
    , gpio_num_t rxPin
    , const char * &err
    , bool isDebug
)
{
    if (command != nullptr && uart_write_bytes (uartNum, command, cmdSize) < 0) {
        err = "uart_write_bytes failed";
        ESP_LOGE (tag, "%s", err);
        return ESP_FAIL;
    }

    int len = uart_read_bytes (uartNum, buf, cmdSize, pdMS_TO_TICKS (200));
    if (len < 0) {
        err = "uart_read_bytes failed";
        ESP_LOGE (tag, "%s", err);
        return ESP_FAIL;
    }

    if (len != cmdSize) {
        sprintf (errorMsg, "Wrong response received: expected %d bytes, got %d bytes", cmdSize, len);
        err = errorMsg;
        ESP_LOGE (tag, "%s", err);
        return ESP_FAIL;
    }

    uint8_t checksum = buf[0] + buf[1] + buf[2] + buf[3] + buf[4] + buf[5] + buf[6] + buf[7] + buf[8];

    if (checksum != 0xff) {
        sprintf (errorMsg, "Checksum mismatch: calculated %02x, expected 0xFF, data %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", checksum, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8]);
        err = errorMsg;
        ESP_LOGE (tag, "%s", err);
        return ESP_FAIL;
    }

    if (isDebug) {
        sprintf (errorMsg, "Raw: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8]);
        err = errorMsg;
        ESP_LOGI (tag, "%s", err);
    }
    else {
        err = nullptr;
    }

    return ESP_OK;
}

// ===================================================================================================
const char * const Mhz19UartImpl :: tag = "Mhz19Uart";
const uint8_t  Mhz19UartImpl :: requestVal[cmdSize] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
const uint8_t  Mhz19UartImpl :: requestCalibration[cmdSize] = {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78};
const uint8_t  Mhz19UartImpl :: requestAlarm[cmdSize] = {0xFF, 0x01, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7A};