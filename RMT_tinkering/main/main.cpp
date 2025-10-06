#include "esp_log.h"
#include "esp_check.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "driver/gpio.h"

#define RMT_GPIO GPIO_NUM_5
#define LOG_GPIO GPIO_NUM_18

const char * const tag = "main";

#define LOG_GPIO_CHANGE  { log_state = !log_state; gpio_set_level (LOG_GPIO, log_state); }

esp_err_t tryRmt()
{
    // --------------------------------------------------------------------------
    // Set up LOG_GPIO
    
    bool log_state = false;

    gpio_config_t log_io_conf = {
        .pin_bit_mask = (1ULL << LOG_GPIO), 
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_RETURN_ON_ERROR (gpio_config (&log_io_conf), tag, "gpio_config failed");

    // --------------------------------------------------------------------------
    // Set up copy encoder
    
    rmt_encoder_handle_t copy_encoder = nullptr;
    rmt_copy_encoder_config_t cec;

    ESP_RETURN_ON_ERROR (rmt_new_copy_encoder (&cec, &copy_encoder), tag, "create copy encoder failed");

    LOG_GPIO_CHANGE;

    // --------------------------------------------------------------------------
    // Set up RX channel
    rmt_rx_channel_config_t rx_chan_config = {
        .gpio_num          = RMT_GPIO,
        .clk_src           = RMT_CLK_SRC_DEFAULT,
        .resolution_hz     = 1'000'000,
        .mem_block_symbols = 64,
        .intr_priority     = 0,
        .flags = {
            .invert_in    = false,
            .with_dma     = false,
            .io_loop_back = false,
            .allow_pd     = false,
        },
    };

    rmt_channel_handle_t rx_chan = nullptr;

    ESP_RETURN_ON_ERROR (rmt_new_rx_channel (&rx_chan_config, &rx_chan), tag, "create rx channel failed");

    LOG_GPIO_CHANGE;

    // --------------------------------------------------------------------------
    // Set up TX channel

    rmt_tx_channel_config_t tx_chan_config = {
        .gpio_num          = RMT_GPIO,
        .clk_src           = RMT_CLK_SRC_DEFAULT,
        .resolution_hz     = 1'000'000,
        .mem_block_symbols = 64,
        .trans_queue_depth = 1,
        .intr_priority     = 0,
        .flags = {
            .invert_out   = true,
            .with_dma     = false,
            .io_loop_back = false,
            .io_od_mode   = true,
            .allow_pd     = false, 
        },
    };

    rmt_channel_handle_t tx_chan = nullptr;

    ESP_RETURN_ON_ERROR (rmt_new_tx_channel (&tx_chan_config, &tx_chan), tag, "create tx channel failed");

    LOG_GPIO_CHANGE;

    // --------------------------------------------------------------------------
    // Enable pullup on GPIO

    ESP_RETURN_ON_ERROR (gpio_set_pull_mode (RMT_GPIO, GPIO_PULLUP_ONLY), tag, "set pull mode failed");

    LOG_GPIO_CHANGE;

    // --------------------------------------------------------------------------
    // Enable TX channel

    ESP_RETURN_ON_ERROR (rmt_enable(tx_chan), tag, "enable tx channel failed");

    LOG_GPIO_CHANGE;

    // --------------------------------------------------------------------------
    // Transmit START (inverted LOW) signal for 1 second

    rmt_symbol_word_t stay_high = {
        .duration0 = 1'000, 
        .level0 = 1,
        .duration1 = 0, 
        .level1 = 0, 
    };

    rmt_transmit_config_t transmit_config = {
        .loop_count = 0,
        .flags = {
            .eot_level = 0,
            .queue_nonblocking = false,
        }
    };
    
    ESP_RETURN_ON_ERROR (rmt_transmit(tx_chan, copy_encoder, &stay_high, sizeof(stay_high), &transmit_config), tag, "transmit failed");
    
    LOG_GPIO_CHANGE;

    return ESP_OK;
}

extern "C" void app_main(void)
{
    ESP_LOGI (tag, "Start");

    esp_err_t rc = tryRmt();

    if (rc != ESP_OK) {
        ESP_LOGE (tag, "RMT test failed: %d", rc);
    } 
    else {
        ESP_LOGI (tag, "RMT test succeeded");
    }

    ESP_LOGI (tag, "Finish");
}