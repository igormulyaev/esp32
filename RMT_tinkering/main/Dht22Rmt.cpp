#include "Dht22Rmt.hpp"
#include "esp_check.h"

// ===================================================================================================
esp_err_t Dht22Rmt :: init (gpio_num_t dht22GpioNum)
{
    // --------------------------------------------------------------------------
    // Set up copy encoder
    rmt_copy_encoder_config_t cec;
    ESP_RETURN_ON_ERROR (rmt_new_copy_encoder (&cec, &copyEncoder), tag, "create copy encoder failed");

    // --------------------------------------------------------------------------
    // Set up RX channel
    rmt_rx_channel_config_t rxChCfg = {
        .gpio_num          = dht22GpioNum,
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
    ESP_RETURN_ON_ERROR (rmt_new_rx_channel (&rxChCfg, &rxChannel), tag, "create rx channel failed");

    // --------------------------------------------------------------------------
    // Set up TX channel
    rmt_tx_channel_config_t txChCfg = {
        .gpio_num          = dht22GpioNum,
        .clk_src           = RMT_CLK_SRC_DEFAULT,
        .resolution_hz     = 1'000'000,
        .mem_block_symbols = 64,
        .trans_queue_depth = 1,
        .intr_priority     = 0,
        .flags = {
            .invert_out   = true, // Use inverted output to avoid after-setup LOW pulse
            .with_dma     = false,
            .io_loop_back = false,
            .io_od_mode   = true, // Open-drain mode for I/O on the same pin
            .allow_pd     = false, 
        },
    };
    ESP_RETURN_ON_ERROR (rmt_new_tx_channel (&txChCfg, &txChannel), tag, "create tx channel failed");

    // --------------------------------------------------------------------------
    // Enable pullup on GPIO
    ESP_RETURN_ON_ERROR (gpio_set_pull_mode (dht22GpioNum, GPIO_PULLUP_ONLY), tag, "set pull mode failed");

    // --------------------------------------------------------------------------
    // Create receive queue
    receiveQueue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));
    ESP_RETURN_ON_FALSE (receiveQueue != nullptr, ESP_ERR_NO_MEM, tag, "receive queue creation failed");

    // --------------------------------------------------------------------------
    // Register rmt rx done callback
    rmt_rx_event_callbacks_t rxCallbackConfig = {
        .on_recv_done = rmtRxDoneCallback,
    };
    ESP_RETURN_ON_ERROR (rmt_rx_register_event_callbacks (rxChannel, &rxCallbackConfig, this), tag, "enable rmt rx channel failed");

    // --------------------------------------------------------------------------
    // Enable TX and RX channels

    ESP_RETURN_ON_ERROR (rmt_enable (rxChannel), tag, "enable rx channel failed");
    ESP_RETURN_ON_ERROR (rmt_enable (txChannel), tag, "enable tx channel failed");

    return ESP_OK;
}

// ===================================================================================================
esp_err_t Dht22Rmt :: deinit ()
{
    // Implementation of deinitialization logic for DHT22 using RMT
    return ESP_OK;
}

// ===================================================================================================
esp_err_t Dht22Rmt :: readData (int &temperatureX10, int &humidityX10)
{
    ESP_RETURN_ON_ERROR (rmt_transmit (txChannel, copyEncoder, &startSymbol, sizeof(startSymbol), &transmitConfig), tag, "transmit failed");
    
    ESP_RETURN_ON_ERROR (rmt_receive (rxChannel, rxBuffer, sizeof(rxBuffer), &receiveConfig), tag, "receive failed");    
    
    rmt_rx_done_event_data_t rcData;
    ESP_RETURN_ON_FALSE (xQueueReceive (receiveQueue, &rcData, pdMS_TO_TICKS(100)) == pdPASS, ESP_ERR_TIMEOUT, tag, "data receive timeout");

    return decodeDht22 (&rcData, temperatureX10, humidityX10);
}

// ===================================================================================================
esp_err_t Dht22Rmt :: decodeDht22 (const rmt_rx_done_event_data_t *rxData, int &temperatureX10, int &humidityX10)
{
    ESP_RETURN_ON_FALSE (rxData->num_symbols == 43, ESP_FAIL, tag, "Invalid number of symbols: %d != 43", rxData->num_symbols);

    ESP_RETURN_ON_FALSE (
        rxData->received_symbols[0].level0 == 1 
        && rxData->received_symbols[0].duration0 >= 15
        && rxData->received_symbols[0].duration0 < 31
        && rxData->received_symbols[0].level1 == 0
        && rxData->received_symbols[0].duration1 >= 70
        && rxData->received_symbols[0].duration1 < 90
        && rxData->received_symbols[1].level0 == 1
        && rxData->received_symbols[1].duration0 >= 70
        && rxData->received_symbols[1].duration0 < 90
        , ESP_FAIL
        , tag
        , "Invalid start signal"
    );

    uint8_t dataBytes[5] = {0, 0, 0, 0, 0};

    for (size_t i = 2; i < 42; ++i) {
        ESP_RETURN_ON_FALSE (
            rxData->received_symbols[i-1].level1 == 0
            && rxData->received_symbols[i-1].duration1 >= 45
            && rxData->received_symbols[i-1].duration1 < 75
            , ESP_FAIL
            , tag
            , "Invalid start of bit %d"
            , i - 2
        );
        ESP_RETURN_ON_FALSE (
            rxData->received_symbols[i].level1 == 0
            && rxData->received_symbols[i].duration1 >= 20
            && rxData->received_symbols[i].duration1 < 80
            , ESP_FAIL
            , tag
            , "Invalid data of bit %d"
            , i - 2
        );
        uint8_t bit_value = (rxData->received_symbols[i].duration0 > 50) ? 1 : 0;
        dataBytes[(i - 2) / 8] <<= 1;
        dataBytes[(i - 2) / 8] |= bit_value;
    }

    ESP_RETURN_ON_FALSE (
       ((dataBytes[0] + dataBytes[1] + dataBytes[2] + dataBytes[3] - dataBytes[4]) & 0xFF) == 0
        , ESP_FAIL
        , tag
        , "Checksum error: %02X + %02X + %02X + %02X - %02X = %02X != 0"
        , dataBytes[0]
        , dataBytes[1]
        , dataBytes[2]
        , dataBytes[3]
        , dataBytes[4]
        , (dataBytes[0] + dataBytes[1] + dataBytes[2] + dataBytes[3] - dataBytes[4]) & 0xFF
    );

    humidityX10 = (dataBytes[0] << 8) | dataBytes[1];
    temperatureX10 = (((dataBytes[2] & 0x7f) << 8) | dataBytes[3]) * ((dataBytes[2] & 0x80) == 0 ? 1 : -1);

    return ESP_OK;
}

// ===================================================================================================
IRAM_ATTR
bool Dht22Rmt :: rmtRxDoneCallback (rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data)
{
    Dht22Rmt * pThis = static_cast<Dht22Rmt *> (user_data);
    BaseType_t taskWoken = pdFALSE;
    return xQueueSendFromISR (pThis -> receiveQueue, edata, &taskWoken) == pdTRUE;
}

// ===================================================================================================
const rmt_symbol_word_t Dht22Rmt :: startSymbol = {
    .duration0 = 1'000, 
    .level0 = 1,
    .duration1 = 0, 
    .level1 = 0, 
};

const rmt_transmit_config_t Dht22Rmt :: transmitConfig = {
    .loop_count = 0,
    .flags = {
        .eot_level = 0,
        .queue_nonblocking = false,
    }
};

const rmt_receive_config_t Dht22Rmt :: receiveConfig = {
    .signal_range_min_ns = 1000,
    .signal_range_max_ns = 100'000,
    .flags = {
        .en_partial_rx = false,
    },
};

const char * const Dht22Rmt :: tag = "Dht22Rmt";