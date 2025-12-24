#ifndef DHT22RMT_HPP
#define DHT22RMT_HPP

#include "esp_err.h"
#include "driver/gpio.h"

#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"


class Dht22Rmt
{
    public:
        Dht22Rmt () = default;
        ~Dht22Rmt () = default;

        esp_err_t init (gpio_num_t dht22GpioNum, bool usePullup = true);
        esp_err_t deinit ();
        esp_err_t readData (int &temperatureX10, int &humidityX10);
    
    private:
        static bool rmtRxDoneCallback (rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data);
        static esp_err_t decodeDht22 (const rmt_rx_done_event_data_t *rxData, int &temperatureX10, int &humidityX10);

        gpio_num_t dht22GpioNum = GPIO_NUM_NC;
        bool usePullup = false;

        rmt_encoder_handle_t copyEncoder = nullptr;
        rmt_channel_handle_t rxChannel = nullptr;
        rmt_channel_handle_t txChannel = nullptr;
        QueueHandle_t receiveQueue = nullptr;

        rmt_symbol_word_t rxBuffer [48 * 8];

        static const char * const tag;
        static const rmt_symbol_word_t startSymbol;
        static const rmt_transmit_config_t transmitConfig;
        static const rmt_receive_config_t receiveConfig;

        Dht22Rmt (const Dht22Rmt &) = delete;
        Dht22Rmt & operator= (const Dht22Rmt &) = delete;
};

#endif  // DHT22RMT_HPP