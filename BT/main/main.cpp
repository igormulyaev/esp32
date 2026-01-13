#include "testBle.hpp"

#include "esp_check.h"
#include "nvs_flash.h"

static const char * const tag = "main";

// ===============================================================================
static esp_err_t nvsInit () 
{
    esp_err_t rc = nvs_flash_init();

    if (rc == ESP_ERR_NVS_NO_FREE_PAGES || rc == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_RETURN_ON_ERROR(nvs_flash_erase(), tag, "erasing nvs flash failed");
        rc = nvs_flash_init();
    }
    
    return rc;
}

// ===============================================================================
extern "C"
void app_main () 
{
    ESP_LOGI (tag, "starting app_main");

    ESP_RETURN_VOID_ON_ERROR(nvsInit(), tag, "nvs flash init failed");

    ESP_LOGI (tag, "NVS initialized, starting BLE test");
    
    testBle();
}
