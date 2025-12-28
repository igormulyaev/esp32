#include "testBle.hpp"

#include "esp_check.h"

#include "host/ble_hs.h"
#include "host/util/util.h"
//#include "nimble/ble.h"
#include "nimble/nimble_port.h"
//#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"

#define BLE_GAP_APPEARANCE_GENERIC_TAG 0x0200
#define BLE_GAP_LE_ROLE_PERIPHERAL 0x00

static const char * const tag = "testBle";
static const char * const deviceName = "testBle_Beacon";

extern "C" void ble_store_config_init(void);


// ===============================================================================
static esp_err_t gapInit ()
{
    ESP_LOGI (tag, "gapInit started");

    ble_svc_gap_init();

    ESP_RETURN_ON_FALSE(ble_svc_gap_device_name_set (deviceName) == 0, ESP_FAIL, tag, "failed to set device name");

    ESP_RETURN_ON_FALSE(ble_svc_gap_device_appearance_set (BLE_GAP_APPEARANCE_GENERIC_TAG) == 0, ESP_FAIL, tag, "failed to set device appearance");

    return ESP_OK;
}

// ===============================================================================
static void advInit () 
{
    ESP_LOGI (tag, "advInit started");

    // Make sure we have proper BT identity address set
    ESP_RETURN_VOID_ON_FALSE(ble_hs_util_ensure_addr(0) == 0, ESP_FAIL, tag, "device does not have any available bt address");

    // Figure out BT address to use while advertising
    uint8_t ownAddrType;
    ESP_RETURN_VOID_ON_FALSE(ble_hs_id_infer_auto (0, &ownAddrType) == 0, ESP_FAIL, tag, "failed to infer address type");

    // Copy device address to addrVal
    uint8_t addrVal[6];
    ESP_RETURN_VOID_ON_FALSE(ble_hs_id_copy_addr(ownAddrType, addrVal, nullptr) == 0, ESP_FAIL, tag, "failed to copy device address");

    ESP_LOGI(tag, "device address: %02x:%02x:%02x:%02x:%02x:%02x", addrVal[0], addrVal[1], addrVal[2], addrVal[3], addrVal[4], addrVal[5]);

    // Set advertisement data
    ble_hs_adv_fields advFields = {0};

    advFields.name = (const uint8_t *)deviceName;
    advFields.name_len = strlen (deviceName);
    advFields.name_is_complete = 1;
    
    advFields.appearance = BLE_GAP_APPEARANCE_GENERIC_TAG;
    advFields.appearance_is_present = 1;
    
    advFields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    
    advFields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    advFields.tx_pwr_lvl_is_present = 1;
    
    advFields.le_role = BLE_GAP_LE_ROLE_PERIPHERAL;
    advFields.le_role_is_present = 1;

    ESP_RETURN_VOID_ON_FALSE (ble_gap_adv_set_fields(&advFields) == 0, ESP_FAIL, tag, "failed to set advertising data");

    // Set scan response data
    ble_hs_adv_fields rspFields = {0};
    
    rspFields.device_addr = addrVal;
    rspFields.device_addr_type = ownAddrType;
    rspFields.device_addr_is_present = 1;

    //rspFields.uri = esp_uri;
    //rspFields.uri_len = sizeof(esp_uri);

    ESP_RETURN_VOID_ON_FALSE (ble_gap_adv_rsp_set_fields (&rspFields) == 0, ESP_FAIL, tag, "failed to set scan response data");

    // Set non-connectable and general discoverable mode to be a beacon
    ble_gap_adv_params advParams = {0};
    
    advParams.conn_mode = BLE_GAP_CONN_MODE_NON;
    advParams.disc_mode = BLE_GAP_DISC_MODE_GEN;

    // Start advertising
    ESP_RETURN_VOID_ON_FALSE (ble_gap_adv_start(ownAddrType, nullptr, BLE_HS_FOREVER, &advParams, nullptr, nullptr) == 0, ESP_FAIL, tag, "failed to start advertising");

    ESP_LOGI (tag, "advertising started");
}

// ===============================================================================
// It's called when host resets BLE stack due to errors
static void onStackReset(int reason) 
{
    ESP_LOGI(tag, "nimble stack reset, reset reason: %d", reason);
}

// ===============================================================================
// It's called when host has synced with controller
static void onStackSync() 
{
    // do advertising initialization
    advInit();
}

// ===============================================================================
static esp_err_t nimbleInit () 
{
    ESP_LOGI (tag, "nimbleInit started");
    
    ESP_RETURN_ON_ERROR (nimble_port_init(), tag, "nimble port init failed");

    ESP_LOGI (tag, "nimble_port_init done, proceed to gapInit");

    ESP_RETURN_ON_ERROR (gapInit(), tag, "gap service init failed");

    ESP_LOGI (tag, "gapInit done, proceed to set host callbacks");


    // Set host callbacks
    ble_hs_cfg.reset_cb = onStackReset;
    ble_hs_cfg.sync_cb = onStackSync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    // Store host configuration
    ble_store_config_init();

    return ESP_OK;
}

// ===============================================================================
void testBle() 
{
    ESP_LOGI (tag, "testBle started");
    
    ESP_RETURN_VOID_ON_ERROR(nimbleInit(), tag, "nimble stack init failed");
    
    ESP_LOGI (tag, "nimble stack initialized, running main loop");

    nimble_port_run();
}
