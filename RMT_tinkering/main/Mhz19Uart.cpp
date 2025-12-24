#include "Mhz19Uart.hpp"
#include "esp_check.h"

// ===================================================================================================
esp_err_t Mhz19Uart :: init(uart_port_t uartNum, int txPin, int rxPin)
{
    this -> uartNum = uartNum;
    this -> txPin = txPin;
    this -> rxPin = rxPin;

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
esp_err_t Mhz19Uart :: deinit ()
{
    return ESP_OK;
}

// ===================================================================================================
esp_err_t Mhz19Uart :: readCo2 ()
{
    ESP_LOGI (tag, "Sending request");
    ESP_RETURN_ON_FALSE (uart_write_bytes (uartNum, request, sizeof (request)) >= 0, ESP_FAIL, tag, "uart_write_bytes failed");

    uint8_t response[9];
    int len = uart_read_bytes (uartNum, response, sizeof (response), pdMS_TO_TICKS (200));
    ESP_RETURN_ON_FALSE (len >= 0, ESP_FAIL, tag, "uart_read_bytes failed");

    if (len == sizeof(response)) {
        ESP_LOGI (tag, "Response received: %02x %02x %02x %02x %02x %02x %02x %02x %02x",
            response[0], response[1], response[2], response[3], response[4],
            response[5], response[6], response[7], response[8]
        );
        uint8_t checksum = response[0] + response[1] + response[2] + response[3] + response[4] + response[5] + response[6] + response[7] + response[8];

        ESP_RETURN_ON_FALSE (checksum == 0xff, ESP_FAIL, tag, "checksum mismatch");

        int co2 = (response[2] << 8) | response[3];
        ESP_LOGI (tag, "CO2 Concentration: %d ppm", co2);
        if (response[4] != 0) {
            int temperature = response[4] - 40;
            ESP_LOGI (tag, "Temperature: %d C", temperature);
        }
    } 
    else {
        ESP_LOGW (tag, "Incomplete response received: expected %d bytes, got %d bytes", sizeof(response), len);
    }

    return ESP_OK;
}

// ===================================================================================================
const char * const Mhz19Uart :: tag = "Mhz19Uart";
const uint8_t  Mhz19Uart :: request[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};