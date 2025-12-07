#include "esp_log.h"
#include "esp_check.h"

#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "Dht22Rmt.hpp"

#define RMT_GPIO GPIO_NUM_5
/*
#define MAIN_LOG_GPIO GPIO_NUM_18
#define RMT_LOG_GPIO GPIO_NUM_19

bool write_log_state = false;
bool read_log_state = false;

#define MAIN_LOG_GPIO_CHANGE { write_log_state = !write_log_state; gpio_set_level (MAIN_LOG_GPIO, write_log_state); }
#define RMT_LOG_GPIO_CHANGE { read_log_state = !read_log_state; gpio_set_level (RMT_LOG_GPIO, read_log_state); }
*/

const char * const tag = "main";

Dht22Rmt dht22rmt;

// ==========================================================================
extern "C" void app_main(void)
{
    ESP_LOGI (tag, "Start");

    for (int i = 2; i != 0; --i) {
        if (dht22rmt.init (RMT_GPIO) == ESP_OK) {
            ESP_LOGI (tag, "DHT22 RMT initialized");
            int temperatureX10 = 0;
            int humidityX10 = 0;

            for (int j = 2; j != 0; --j) {
                if (dht22rmt.readData (temperatureX10, humidityX10) != ESP_OK) {
                    break;
                }
                ESP_LOGI (tag, "Temperature: %d.%d C, Humidity: %d.%d %%", temperatureX10 / 10, temperatureX10 % 10, humidityX10 / 10, humidityX10 % 10);
                
                vTaskDelay (pdMS_TO_TICKS(2000));
            }
        }

        esp_err_t rc = dht22rmt.deinit();
        if (rc != ESP_OK) {
            ESP_LOGE (tag, "DHT22 RMT deinit failed: %s", esp_err_to_name (rc));
            break;
        } 
        else {
            ESP_LOGI (tag, "DHT22 RMT deinitialized");
        }
    }
    ESP_LOGI (tag, "Finish");
}