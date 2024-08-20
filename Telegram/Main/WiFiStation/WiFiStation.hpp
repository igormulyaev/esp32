#ifndef WIFISTATION_HPP
#define WIFISTATION_HPP

#include "esp_event.h"
#include "EventGroup.hpp"

class WiFiStation
{
    public:
        WiFiStation() {};
        ~WiFiStation() {};

        void Init();
        void Run();

        void WaitForConnect();
        void WaitForDisconnect();
    
    private:
        EventGroup event;
        bool isConnected;

        void OnConnect();
        void OnDisconnect();
        
        enum 
        {
            evConnect = BIT0
            , evAckConnect = BIT1
            , evDisconnect = BIT2
            , evAckDisconnect = BIT3
        };

        static void WiFiEventHandler (void* p, esp_event_base_t event_base, int32_t event_id, void* event_data);
        static void IpEventHandler   (void* p, esp_event_base_t event_base, int32_t event_id, void* event_data);

        static const char * const TAG;
};

#endif