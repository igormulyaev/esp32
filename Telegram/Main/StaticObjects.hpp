#ifndef STATICOBJECTS_HPP
#define STATICOBJECTS_HPP

class NVS;
class WiFiStation;
class TelegramTask;
class Console;

extern NVS nvs;
extern WiFiStation sta;
extern TelegramTask tgTask;
extern Console console;

extern const char cert_pem_start[] asm("_binary_telegram_org_pem_start");

extern const char * const tgBotId;
extern const char * const tgBotKey;
extern const char * const WifiSsid;
extern const char * const WifiPassword;

#endif