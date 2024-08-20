#include "NVS.hpp"
#include "nvs_flash.h"
#include "esp_log.h"

void NVS :: Init()
{
    ESP_LOGI(TAG, "Begin Init");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_LOGI(TAG, "End Init");
}

const char * const NVS :: TAG = "NVS";
