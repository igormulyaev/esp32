#include "esp_log.h"
#include "esp_check.h"

#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "Dht22Rmt.hpp"
#include "Mhz19Uart.hpp"

#define RMT_GPIO GPIO_NUM_5

static const char * const tag = "main";

Dht22Rmt dht22rmt;

class Mhz19Uart_1_17_16 : public Mhz19Uart <Mhz19Uart_1_17_16>
{
    public:
        static constexpr uart_port_t uartNum = UART_NUM_1;
        static constexpr gpio_num_t txPin = GPIO_NUM_17;
        static constexpr gpio_num_t rxPin = GPIO_NUM_16;
};

Mhz19Uart_1_17_16 mhz19uart;

// ==========================================================================
void testDht22 () 
{
    if (dht22rmt.init (RMT_GPIO) == ESP_OK) {
        ESP_LOGI (tag, "DHT22 RMT initialized");
        int temperatureX10 = 0;
        int humidityX10 = 0;

        for (int j = 3; j != 0; --j) {
            if (dht22rmt.readData (temperatureX10, humidityX10) != ESP_OK) {
                break;
            }
            ESP_LOGI (tag, "Temperature: %d.%d C, Humidity: %d.%d %%", temperatureX10 / 10, temperatureX10 % 10, humidityX10 / 10, humidityX10 % 10);
            
            vTaskDelay (pdMS_TO_TICKS(2000));
        }
    }

    esp_err_t rc = dht22rmt.deinit ();
    if (rc != ESP_OK) {
        ESP_LOGE (tag, "DHT22 RMT deinit failed: %s", esp_err_to_name (rc));
    } 
    else {
        ESP_LOGI (tag, "DHT22 RMT deinitialized");
    }
}

// ==========================================================================
void testMhz19Uart ()
{
    if (mhz19uart.init() == ESP_OK) {
        ESP_LOGI (tag, "MH-Z19 UART initialized");

        int co2 = 0;
        int temperature = 0;
        while (true) {
            if (mhz19uart.readCo2 (co2, temperature) != ESP_OK) {
                break;
            }
            ESP_LOGI (tag, "CO2 Concentration: %d ppm, Temperature: %d C", co2, temperature);
            vTaskDelay (pdMS_TO_TICKS(30000));
        }
    }

    esp_err_t rc = mhz19uart.deinit ();
    if (rc != ESP_OK) {
        ESP_LOGE (tag, "MH-Z19 UART deinit failed: %s", esp_err_to_name (rc));
    } 
    else {
        ESP_LOGI (tag, "MH-Z19 UART deinitialized");
    }
}

// ==========================================================================
extern "C" 
void app_main ()
{
    ESP_LOGI (tag, "Start");

    esp_reset_reason_t resetReason = esp_reset_reason();

    if (resetReason == ESP_RST_POWERON || resetReason == ESP_RST_SW) {
        //testDht22 ();
        testMhz19Uart ();
    }
    else {
        ESP_LOGI (tag, "Stop processing due to reset reason %d", resetReason);
    }

    ESP_LOGI (tag, "Finish");
}