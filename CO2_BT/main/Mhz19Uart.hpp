#ifndef MHZ19_UART_HPP
#define MHZ19_UART_HPP

#include "driver/uart.h"
#include "soc/gpio_num.h"

class Mhz19UartImpl
{
    public:
        Mhz19UartImpl() = default;
        ~Mhz19UartImpl() = default;

        static esp_err_t init(uart_port_t uartNum, gpio_num_t txPin, gpio_num_t rxPin);
        static esp_err_t deinit (uart_port_t uartNum, gpio_num_t txPin, gpio_num_t rxPin);

        esp_err_t readCo2 (int &co2, int &temperature, uart_port_t uartNum, gpio_num_t txPin, gpio_num_t rxPin, const char * &err, bool isDebug = false);

    private:
        static const char * const tag;
        static constexpr int baudRate = 9600;
        static const uint8_t request[9];

        char errorMsg[128];

        Mhz19UartImpl (const Mhz19UartImpl &) = delete;
        Mhz19UartImpl & operator= (const Mhz19UartImpl &) = delete;
};

template<class T>
class Mhz19Uart 
{
    public:
        Mhz19Uart() = default;
        ~Mhz19Uart() = default;

        esp_err_t init() 
        {
            T *t = static_cast<T*>(this);
            return impl.init(t->uartNum, t->txPin, t->rxPin);
        }
        esp_err_t deinit ()
        {
            T *t = static_cast<T*>(this);
            return impl.deinit(t->uartNum, t->txPin, t->rxPin);
        }

        esp_err_t readCo2 (int &co2, int &temperature, const char * &err, bool isDebug = false)
        {
            T *t = static_cast<T*>(this);
            return impl.readCo2(co2, temperature, t->uartNum, t->txPin, t->rxPin, err, isDebug);
        }

    private:
        Mhz19UartImpl impl;
        
        Mhz19Uart (const Mhz19Uart &) = delete;
        Mhz19Uart& operator= (const Mhz19Uart &) = delete;
};

#endif // MHZ19_UART_HPP