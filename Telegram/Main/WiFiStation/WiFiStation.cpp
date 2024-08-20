#include "WiFiStation.hpp"
#include "esp_wifi.h"
#include "esp_log.h"
#include "../StaticObjects.hpp"
#include <cstring>
// ----------------------------------------------------------------------------------
void WiFiStation :: Init()
{
    ESP_LOGI (TAG, "Begin Init");
    
    event.Init();

    ESP_ERROR_CHECK (esp_netif_init());

    ESP_ERROR_CHECK (esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK (esp_wifi_init (&cfg));

    esp_event_handler_instance_t instance_any_id;

    ESP_ERROR_CHECK (esp_event_handler_instance_register (
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        WiFiEventHandler,
        this,
        &instance_any_id
    ));

    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK (esp_event_handler_instance_register (
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        IpEventHandler,
        this,
        &instance_got_ip
    ));

    ESP_ERROR_CHECK (esp_wifi_set_mode (WIFI_MODE_STA));

    ESP_LOGI (TAG, "End Init");
}

// ----------------------------------------------------------------------------------
void WiFiStation :: Run()
{
    ESP_LOGI (TAG, "Begin Run");

    isConnected = false;
    
    wifi_config_t wifi_config;

    strcpy ((char *) wifi_config.sta.ssid, WifiSsid);
    strcpy ((char *) wifi_config.sta.password, WifiPassword);

    ESP_ERROR_CHECK (esp_wifi_set_config (WIFI_IF_STA, &wifi_config));

    ESP_ERROR_CHECK (esp_wifi_start());

    ESP_LOGI (TAG, "End Run");
}

// ----------------------------------------------------------------------------------
void WiFiStation :: WiFiEventHandler (void *p, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    //WiFiStation* wfs = static_cast <WiFiStation*> (p);

    switch (event_id)
    {
        case WIFI_EVENT_STA_START:
            ESP_LOGI (TAG, "Connect");
            ESP_ERROR_CHECK (esp_wifi_connect());
            break;
    }
    return;
}

// ----------------------------------------------------------------------------------
void WiFiStation :: IpEventHandler (void *p, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    WiFiStation* wfs = static_cast <WiFiStation*> (p);

    switch (event_id)
    {
        case IP_EVENT_STA_GOT_IP:
            {
                ip_event_got_ip_t* eventIp4 = static_cast<ip_event_got_ip_t*> (event_data);
                ESP_LOGI (TAG, "got IPv4: " IPSTR, IP2STR (&eventIp4->ip_info.ip));
                wfs -> OnConnect();
            }
            break;

        case IP_EVENT_GOT_IP6:
            {
                ip_event_got_ip6_t* eventIp6 = static_cast<ip_event_got_ip6_t*> (event_data);
                ESP_LOGI (TAG, "got IPv6: " IPV6STR, IPV62STR(eventIp6->ip6_info.ip));
                wfs -> OnConnect();
            }
            break;
    }
    return;
}

// ----------------------------------------------------------------------------------
void WiFiStation :: OnConnect()
{
    if (!isConnected)
    {
        isConnected = true;
        event.SetBits (evConnect);
        event.Wait (evAckConnect);
    }
}

// ----------------------------------------------------------------------------------
void WiFiStation :: OnDisconnect()
{
    if (isConnected)
    {
        isConnected = false;
        event.SetBits (evDisconnect);
        event.Wait (evAckDisconnect);
    }
}

// ----------------------------------------------------------------------------------
void WiFiStation :: WaitForConnect()
{
    event.Wait (evConnect);
    event.SetBits (evAckConnect);
}

// ----------------------------------------------------------------------------------
void WiFiStation :: WaitForDisconnect()
{
    event.Wait (evDisconnect);
    event.SetBits (evAckDisconnect);
}

// ----------------------------------------------------------------------------------
const char * const WiFiStation :: TAG = "WiFiStation";
