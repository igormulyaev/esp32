#include "esp_log.h"
#include "esp_check.h"

#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"


#define RMT_GPIO GPIO_NUM_5
#define MAIN_LOG_GPIO GPIO_NUM_18
#define RMT_LOG_GPIO GPIO_NUM_19

const char * const tag = "main";

bool write_log_state = false;
bool read_log_state = false;
#define MAIN_LOG_GPIO_CHANGE { write_log_state = !write_log_state; gpio_set_level (MAIN_LOG_GPIO, write_log_state); }
#define RMT_LOG_GPIO_CHANGE { read_log_state = !read_log_state; gpio_set_level (RMT_LOG_GPIO, read_log_state); }

rmt_symbol_word_t rx_symbols_buf [64 * 8];

IRAM_ATTR
bool rmt_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data)
{
    RMT_LOG_GPIO_CHANGE;

    QueueHandle_t receive_queue = static_cast<QueueHandle_t> (user_data);
    BaseType_t task_woken = pdFALSE;
    xQueueSendFromISR (receive_queue, edata, &task_woken);

    return task_woken;
}

esp_err_t setupLogGpio (gpio_num_t gpio_num)
{
    gpio_config_t log_io_conf = {
        .pin_bit_mask = (1ULL << gpio_num), 
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    return gpio_config (&log_io_conf);
}

esp_err_t DHT22Decoder (const rmt_rx_done_event_data_t *rx_data)
{
    ESP_RETURN_ON_FALSE (rx_data->num_symbols == 43, ESP_FAIL, tag, "Invalid number of symbols: %d != 43", rx_data->num_symbols);

    ESP_RETURN_ON_FALSE (
        rx_data->received_symbols[0].level0 == 1 
        && rx_data->received_symbols[0].duration0 >= 15
        && rx_data->received_symbols[0].duration0 < 31
        && rx_data->received_symbols[0].level1 == 0
        && rx_data->received_symbols[0].duration1 >= 70
        && rx_data->received_symbols[0].duration1 < 90
        && rx_data->received_symbols[1].level0 == 1
        && rx_data->received_symbols[1].duration0 >= 70
        && rx_data->received_symbols[1].duration0 < 90
        , ESP_FAIL
        , tag
        , "Invalid start signal"
    );

    uint8_t data_bytes[5] = {0, 0, 0, 0, 0};

    for (size_t i = 2; i < 42; ++i) {
        ESP_RETURN_ON_FALSE (
            rx_data->received_symbols[i-1].level1 == 0
            && rx_data->received_symbols[i-1].duration1 >= 45
            && rx_data->received_symbols[i-1].duration1 < 75
            , ESP_FAIL
            , tag
            , "Invalid start of bit %d"
            , i - 2
        );
        ESP_RETURN_ON_FALSE (
            rx_data->received_symbols[i].level1 == 0
            && rx_data->received_symbols[i].duration1 >= 20
            && rx_data->received_symbols[i].duration1 < 80
            , ESP_FAIL
            , tag
            , "Invalid data of bit %d"
            , i - 2
        );
        uint8_t bit_value = (rx_data->received_symbols[i].duration0 > 50) ? 1 : 0;
        data_bytes[(i - 2) / 8] <<= 1;
        data_bytes[(i - 2) / 8] |= bit_value;
    }

    ESP_RETURN_ON_FALSE (
       ((data_bytes[0] + data_bytes[1] + data_bytes[2] + data_bytes[3] - data_bytes[4]) & 0xFF) == 0
        , ESP_FAIL
        , tag
        , "Checksum error: %02X + %02X + %02X + %02X - %02X = %02X != 0"
        , data_bytes[0]
        , data_bytes[1]
        , data_bytes[2]
        , data_bytes[3]
        , data_bytes[4]
        , (data_bytes[0] + data_bytes[1] + data_bytes[2] + data_bytes[3] - data_bytes[4]) & 0xFF
    );

    unsigned int humidity = (data_bytes[0] << 8) | data_bytes[1];
    unsigned int temperature = (((data_bytes[2] & 0x7f) << 8) | data_bytes[3]) * ((data_bytes[2] & 0x80) == 0 ? 1 : -1);

    ESP_LOGI (tag, "Humidity: %d.%d %%, Temperature: %d.%d C", humidity / 10, humidity % 10, temperature / 10, temperature % 10);

    return ESP_OK;
}

esp_err_t tryRmt()
{
    // --------------------------------------------------------------------------
    // Setup log gpios
    ESP_RETURN_ON_ERROR (setupLogGpio (MAIN_LOG_GPIO), tag, "Setup main log gpio failed");
    ESP_RETURN_ON_ERROR (setupLogGpio (RMT_LOG_GPIO), tag, "Setup RMT log gpio failed");
    
    // --------------------------------------------------------------------------
    // Set up copy encoder
    
    rmt_encoder_handle_t copy_encoder = nullptr;
    rmt_copy_encoder_config_t cec;

    ESP_RETURN_ON_ERROR (rmt_new_copy_encoder (&cec, &copy_encoder), tag, "create copy encoder failed");

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

    // --------------------------------------------------------------------------
    // Enable pullup on GPIO

    ESP_RETURN_ON_ERROR (gpio_set_pull_mode (RMT_GPIO, GPIO_PULLUP_ONLY), tag, "set pull mode failed");

    // --------------------------------------------------------------------------
    // Create receive queue
    QueueHandle_t receive_queue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));

    //ESP_RETURN_ON_ERROR (receive_queue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t)), tag, "receive queue creation failed");

    // --------------------------------------------------------------------------
    // Register rmt rx done callback
    rmt_rx_event_callbacks_t rx_callback_cfg = {
        .on_recv_done = rmt_rx_done_callback
    };

    ESP_RETURN_ON_ERROR (rmt_rx_register_event_callbacks (rx_chan, &rx_callback_cfg, receive_queue), tag, "enable rmt rx channel failed");
    
    // --------------------------------------------------------------------------
    // Enable TX and RX channels

    ESP_RETURN_ON_ERROR (rmt_enable (rx_chan), tag, "enable rx channel failed");
    ESP_RETURN_ON_ERROR (rmt_enable (tx_chan), tag, "enable tx channel failed");

    // --------------------------------------------------------------------------
    // Transmit START (inverted LOW) signal for 1 ms

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
    
    const rmt_receive_config_t rmt_rx_config = {
        .signal_range_min_ns = 1000,
        .signal_range_max_ns = 100'000,
        .flags = {
            .en_partial_rx = false,
        },
    };

    MAIN_LOG_GPIO_CHANGE;
    
    ESP_RETURN_ON_ERROR (rmt_transmit (tx_chan, copy_encoder, &stay_high, sizeof(stay_high), &transmit_config), tag, "transmit failed");
    
    MAIN_LOG_GPIO_CHANGE;
    
    ESP_RETURN_ON_ERROR (rmt_receive (rx_chan, rx_symbols_buf, sizeof(rx_symbols_buf), &rmt_rx_config), tag, "receive failed");    
    
    MAIN_LOG_GPIO_CHANGE;
    
    rmt_rx_done_event_data_t rmt_rx_evt_data;

    ESP_RETURN_ON_FALSE (xQueueReceive (receive_queue, &rmt_rx_evt_data, pdMS_TO_TICKS(100)) == pdPASS, ESP_ERR_TIMEOUT, tag, "data receive timeout");
    
    MAIN_LOG_GPIO_CHANGE;
    
    /*ESP_LOGI (tag, "Received %d symbols", rmt_rx_evt_data.num_symbols);
    
    for (size_t i = 0; i < rmt_rx_evt_data.num_symbols; ++i) {
        ESP_LOGI (tag
            , "%d: %c %d / %c %d"
            , i
            , rmt_rx_evt_data.received_symbols[i].level0 ? 'H' : 'L'
            , rmt_rx_evt_data.received_symbols[i].duration0
            , rmt_rx_evt_data.received_symbols[i].level1 ? 'H' : 'L'
            , rmt_rx_evt_data.received_symbols[i].duration1
        );
    }*/

    return DHT22Decoder (&rmt_rx_evt_data);
}

// ==========================================================================
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