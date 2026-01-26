#ifndef MHZ19_UART_HPP
#define MHZ19_UART_HPP

#include "driver/uart.h"
#include "soc/gpio_num.h"

class Mhz19UartImpl
{
    public:
        Mhz19UartImpl() = default;
        ~Mhz19UartImpl() = default;

        static esp_err_t init(uart_port_t uartNum, gpio_num_t txPin, gpio_num_t rxPin, const char * &err);
        static esp_err_t deinit (uart_port_t uartNum, gpio_num_t txPin, gpio_num_t rxPin, const char * &err);

        esp_err_t readCo2 (int &co2, int &temperature, uart_port_t uartNum, gpio_num_t txPin, gpio_num_t rxPin, const char * &err, bool isDebug = false);
        
        esp_err_t calibrateZero (uart_port_t uartNum, gpio_num_t txPin, gpio_num_t rxPin, const char * &res) 
        {
            uint8_t buf[cmdSize];
            return sendCommand (requestCalibration, buf, uartNum, txPin, rxPin, res, true);
        }

        esp_err_t readAlarm (uart_port_t uartNum, gpio_num_t txPin, gpio_num_t rxPin, const char * &res) 
        {
            uint8_t buf[cmdSize];
            return sendCommand (requestAlarm, buf, uartNum, txPin, rxPin, res, true);
        }

    private:
        esp_err_t sendCommand (
            const uint8_t * command
            , uint8_t * buf
            , uart_port_t uartNum
            , gpio_num_t txPin
            , gpio_num_t rxPin
            , const char * &res
            , bool isDebug = false
        );

        static const char * const tag;
        static constexpr int baudRate = 9600;
        static constexpr size_t cmdSize = 9;

        static const uint8_t requestVal[cmdSize];
        static const uint8_t requestCalibration[cmdSize];
        static const uint8_t requestAlarm[cmdSize];

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

        esp_err_t init(const char * &err) 
        {
            T *t = static_cast<T*>(this);
            return impl.init(t->uartNum, t->txPin, t->rxPin, err);
        }
        esp_err_t deinit (const char * &err)
        {
            T *t = static_cast<T*>(this);
            return impl.deinit(t->uartNum, t->txPin, t->rxPin, err);
        }

        esp_err_t readCo2 (int &co2, int &temperature, const char * &err, bool isDebug = false)
        {
            T *t = static_cast<T*>(this);
            return impl.readCo2(co2, temperature, t->uartNum, t->txPin, t->rxPin, err, isDebug);
        }

        esp_err_t calibrateZero (const char * &res)
        {
            T *t = static_cast<T*>(this);
            return impl.calibrateZero(t->uartNum, t->txPin, t->rxPin, res);
        }

        esp_err_t readAlarm (const char * &res)
        {
            T *t = static_cast<T*>(this);
            return impl.readAlarm(t->uartNum, t->txPin, t->rxPin, res);
        }

    private:
        Mhz19UartImpl impl;
        
        Mhz19Uart (const Mhz19Uart &) = delete;
        Mhz19Uart& operator= (const Mhz19Uart &) = delete;
};

#endif // MHZ19_UART_HPP