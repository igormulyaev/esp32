#ifndef MHZ19_UART_HPP
#define MHZ19_UART_HPP

#include "driver/uart.h"

class Mhz19Uart 
{
    public:
        Mhz19Uart() = default;
        ~Mhz19Uart() = default;

        esp_err_t init(uart_port_t uartNum, int txPin, int rxPin);
        esp_err_t deinit ();

        esp_err_t readCo2 ();

    private:
        uart_port_t uartNum;
        int txPin;
        int rxPin;

        static const char * const tag;
        static constexpr int baudRate = 9600;
        static const uint8_t request[9];

        Mhz19Uart (const Mhz19Uart &) = delete;
        Mhz19Uart & operator= (const Mhz19Uart &) = delete;
};

#endif // MHZ19_UART_HPP