#include "Mhz19Uart.hpp"
#include "esp_check.h"

// ===================================================================================================
esp_err_t Mhz19UartImpl :: init(uart_port_t uartNum, gpio_num_t txPin, gpio_num_t rxPin)
{
    ESP_RETURN_ON_ERROR (uart_driver_install (uartNum, 129 /* minimum size, must be > 128 */, 0, 0, nullptr, 0), tag, "uart_driver_install failed");

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

    ESP_RETURN_ON_ERROR (uart_param_config(uartNum, &uartConfig), tag, "uart_param_config failed");

    ESP_RETURN_ON_ERROR (uart_set_pin (uartNum, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE), tag, "uart_set_pin failed");

    return ESP_OK;
}

// ===================================================================================================
esp_err_t Mhz19UartImpl :: deinit (uart_port_t uartNum, gpio_num_t txPin, gpio_num_t rxPin)
{
    return ESP_OK;
}

// ===================================================================================================
esp_err_t Mhz19UartImpl :: readCo2 (int &co2, int &temperature, uart_port_t uartNum, gpio_num_t txPin, gpio_num_t rxPin, const char * &err, bool isDebug)
{
    int rc = uart_write_bytes (uartNum, request, sizeof (request));
    if (rc < 0) {
        err = "uart_write_bytes failed";
        ESP_LOGE (tag, "%s", err);
        return ESP_FAIL;
    }

    uint8_t response[9];
    int len = uart_read_bytes (uartNum, response, sizeof (response), pdMS_TO_TICKS (200));
    if (len < 0) {
        err = "uart_read_bytes failed";
        ESP_LOGE (tag, "%s", err);
        return ESP_FAIL;
    }

    if (len != sizeof(response)) {
        sprintf (errorMsg, "Wrong response received: expected %d bytes, got %d bytes", sizeof(response), len);
        err = errorMsg;
        ESP_LOGE (tag, "%s", err);
        return ESP_FAIL;
    }

    uint8_t checksum = response[0] + response[1] + response[2] + response[3] + response[4] + response[5] + response[6] + response[7] + response[8];

    if (checksum != 0xff) {
        sprintf (errorMsg, "Checksum mismatch: calculated %02x, expected 0xFF, data %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", checksum, response[0], response[1], response[2], response[3], response[4], response[5], response[6], response[7], response[8]);
        err = errorMsg;
        ESP_LOGE (tag, "%s", err);
        return ESP_FAIL;
    }

    if (isDebug) {
        sprintf (errorMsg, "Response: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", response[0], response[1], response[2], response[3], response[4], response[5], response[6], response[7], response[8]);
        err = errorMsg;
        ESP_LOGI (tag, "%s", err);
    }

    co2 = (response[2] << 8) | response[3];
    
    temperature = response[4] != 0 ? response[4] - 40 : INT_MIN;

    return ESP_OK;
}

// ===================================================================================================
const char * const Mhz19UartImpl :: tag = "Mhz19Uart";
const uint8_t  Mhz19UartImpl :: request[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};