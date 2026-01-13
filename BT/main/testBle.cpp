#include "testBle.hpp"

#include "esp_check.h"

#include "host/ble_hs.h"
#include "host/util/util.h"
//#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#define BLE_GAP_APPEARANCE_GENERIC_TAG 0x0200
#define BLE_GAP_LE_ROLE_PERIPHERAL 0x00

static const char * const tag = "testBle";
static const char * const deviceName = "CO2 Sensor";

extern "C" void ble_store_config_init(void);

static int gattHandler (uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int gapHandler (struct ble_gap_event *event, void *arg);

static uint16_t gattReadValHandle;

static const ble_uuid128_t sppChrRxUuid = {
    .u = BLE_UUID_TYPE_128,
    .value = {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x6e}
};

static const ble_uuid128_t sppChrTxUuid = {
    .u = BLE_UUID_TYPE_128,
    .value = {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40, 0x6e}
};

static const ble_gatt_chr_def gattChrDefs[] = {
    {
        // SPP RX
        .uuid = (const ble_uuid_t*) &sppChrRxUuid,
        .access_cb = gattHandler,
        .arg = nullptr,
        .descriptors = nullptr,
        .flags = BLE_GATT_CHR_F_WRITE,
        .min_key_size = 0,
        .val_handle = &gattReadValHandle,
        .cpfd = nullptr,
    },
    {
        // SPP TX
        .uuid = (const ble_uuid_t*) &sppChrTxUuid,
        .access_cb = gattHandler,
        .arg = nullptr,
        .descriptors = nullptr,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
        .min_key_size = 0,
        .val_handle = &gattReadValHandle,
        .cpfd = nullptr,
    },
    {
        // No more characteristics
        .uuid = nullptr,
        .access_cb = nullptr,
        .arg = nullptr,
        .descriptors = nullptr,
        .flags = 0,
        .min_key_size = 0,
        .val_handle = nullptr,
        .cpfd = nullptr,
    }
};

static const ble_uuid128_t sppSvcUuid = {
    .u = BLE_UUID_TYPE_128,
    .value = {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e}
};

static const struct ble_gatt_svc_def gattSvcDefs[] = {
    {
        // Service: SPP
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = (const ble_uuid_t*) &sppSvcUuid,
        .includes = nullptr,
        .characteristics = gattChrDefs,
    },
    {
        // No more services.
        .type = 0,
        .uuid = nullptr,
        .includes = nullptr,
        .characteristics = nullptr,
    },
};

// ===============================================================================
static int gattHandler(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        ESP_LOGI (tag, "gattHandler: Read event, conn_handle = %d, attr_handle = %d", conn_handle, attr_handle);
        break;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        {
            char buf[ctxt->om->om_len + 1];
            memcpy(buf, ctxt->om->om_data, ctxt->om->om_len);
            buf[ctxt->om->om_len] = '\0';
            for (char *p = buf; *p != '\0'; p++) {
                switch (*p) {
                case '\r':
                    *p = '<';
                    break;
                case '\n':
                    *p = '|';
                    break;
                }
            }
            ESP_LOGI (tag, "gattHandler: Data received in write event, conn_handle = %d, attr_handle = %d, len = %d, data = %s", conn_handle, attr_handle, ctxt->om->om_len, buf);
        }
        break;

    default:
        ESP_LOGW (tag, "gattHandler: unhandled GATT operation: %d", ctxt->op);
        break;
    }
    return 0;
}

// ===============================================================================
static esp_err_t gapGattInit ()
{
    ble_svc_gap_init();
    ESP_LOGI (tag, "gapGattInit: GAP service initialized");

    ble_svc_gatt_init();
    ESP_LOGI (tag, "gapGattInit: GATT service initialized");

    int rc;
    ESP_RETURN_ON_FALSE ((rc = ble_gatts_count_cfg (gattSvcDefs)) == 0, ESP_FAIL, tag, "failed to count GATT configuration, rc = %d", rc);
    ESP_LOGI (tag, "gapGattInit: GATT configuration counted");

    ESP_RETURN_ON_FALSE ((rc = ble_gatts_add_svcs(gattSvcDefs)) == 0, ESP_FAIL, tag, "failed to add GATT services, rc = %d", rc);
    ESP_LOGI (tag, "gapGattInit: GATT services added");

    ESP_RETURN_ON_FALSE ((rc = ble_svc_gap_device_name_set (deviceName)) == 0, ESP_FAIL, tag, "failed to set device name, rc = %d", rc);
    ESP_LOGI (tag, "gapGattInit: device name set");

    //ESP_RETURN_ON_FALSE (ble_svc_gap_device_appearance_set (BLE_GAP_APPEARANCE_GENERIC_TAG) == 0, ESP_FAIL, tag, "failed to set device appearance");

    return ESP_OK;
}

// ===============================================================================
static void printConnDesc(const struct ble_gap_conn_desc *desc)
{
    ESP_LOGI (tag, "Connection description:");
    ESP_LOGI (tag, "    handle = %d, our_ota_addr_type = %d, our_ota_addr = %02x:%02x:%02x:%02x:%02x:%02x"
        , desc->conn_handle
        , desc->our_ota_addr.type
        , desc->our_ota_addr.val[5]
        , desc->our_ota_addr.val[4]
        , desc->our_ota_addr.val[3]
        , desc->our_ota_addr.val[2]
        , desc->our_ota_addr.val[1]
        , desc->our_ota_addr.val[0]
    );
    
    ESP_LOGI (tag, "    our_id_addr_type = %d, our_id_addr = %02x:%02x:%02x:%02x:%02x:%02x"
        , desc->our_id_addr.type
        , desc->our_id_addr.val[5]
        , desc->our_id_addr.val[4]
        , desc->our_id_addr.val[3]
        , desc->our_id_addr.val[2]
        , desc->our_id_addr.val[1]
        , desc->our_id_addr.val[0]
    );
    ESP_LOGI (tag, "    peer_ota_addr_type = %d, peer_ota_addr = %02x:%02x:%02x:%02x:%02x:%02x"
        , desc->peer_ota_addr.type
        , desc->peer_ota_addr.val[5]
        , desc->peer_ota_addr.val[4]
        , desc->peer_ota_addr.val[3]
        , desc->peer_ota_addr.val[2]
        , desc->peer_ota_addr.val[1]
        , desc->peer_ota_addr.val[0]
    );
    ESP_LOGI (tag, "    peer_id_addr_type = %d, peer_id_addr = %02x:%02x:%02x:%02x:%02x:%02x"
        , desc->peer_id_addr.type
        , desc->peer_id_addr.val[5]
        , desc->peer_id_addr.val[4]
        , desc->peer_id_addr.val[3]
        , desc->peer_id_addr.val[2]
        , desc->peer_id_addr.val[1]
        , desc->peer_id_addr.val[0]
    );
    ESP_LOGI (tag, "    conn_itvl = %d, conn_latency = %d, supervision_timeout = %d, encrypted = %d, authenticated = %d, bonded = %d"
        , desc->conn_itvl
        , desc->conn_latency
        , desc->supervision_timeout
        , desc->sec_state.encrypted
        , desc->sec_state.authenticated
        , desc->sec_state.bonded
    );
}

// ===============================================================================
static void advInit () 
{
    int rc;
    // Make sure we have proper BT identity address set
    ESP_RETURN_VOID_ON_FALSE((rc = ble_hs_util_ensure_addr (0)) == 0, ESP_FAIL, tag, "device does not have any available bt address, rc = %d", rc);
    ESP_LOGI (tag, "advInit: BT exists");

    // Figure out BT address to use while advertising
    uint8_t ownAddrType;
    ESP_RETURN_VOID_ON_FALSE((rc = ble_hs_id_infer_auto (0, &ownAddrType)) == 0, ESP_FAIL, tag, "failed to infer address type, rc = %d", rc);
    ESP_LOGI (tag, "advInit: got addr type: %d", ownAddrType);

    // Copy device address to addrVal
    uint8_t addrVal[6];
    ESP_RETURN_VOID_ON_FALSE((rc = ble_hs_id_copy_addr (ownAddrType, addrVal, nullptr)) == 0, ESP_FAIL, tag, "failed to copy device address, rc = %d", rc);
    ESP_LOGI(tag, "advInit: device address: %02x:%02x:%02x:%02x:%02x:%02x", addrVal[0], addrVal[1], addrVal[2], addrVal[3], addrVal[4], addrVal[5]);

    // Set advertisement data
    ble_hs_adv_fields advFields;
    memset (&advFields, 0, sizeof(advFields));

    // Discoverability in forthcoming advertisement (general)
    // BLE-only (BR/EDR unsupported).
    advFields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    /*
    advFields.name = (const uint8_t *)deviceName;
    advFields.name_len = strlen (deviceName);
    advFields.name_is_complete = 1;
    */
    advFields.uuids128 = &sppSvcUuid;
    advFields.num_uuids128 = 1;
    advFields.uuids128_is_complete = 1;

    //advFields.appearance = BLE_GAP_APPEARANCE_GENERIC_TAG;
    //advFields.appearance_is_present = 1;
    
    advFields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    advFields.tx_pwr_lvl_is_present = 1;
    
    advFields.le_role = BLE_GAP_LE_ROLE_PERIPHERAL;
    advFields.le_role_is_present = 1;

    ESP_RETURN_VOID_ON_FALSE ((rc = ble_gap_adv_set_fields (&advFields)) == 0, ESP_FAIL, tag, "failed to set advertising data, rc = %d", rc);
    ESP_LOGI (tag, "advInit: advertising fields set");

    // Set scan response data
    ble_hs_adv_fields rspFields;
    memset (&rspFields, 0, sizeof(rspFields));
    
    rspFields.device_addr = addrVal;
    rspFields.device_addr_type = ownAddrType;
    rspFields.device_addr_is_present = 1;

    rspFields.name = (const uint8_t *)deviceName;
    rspFields.name_len = strlen (deviceName);
    rspFields.name_is_complete = 1;

    ESP_RETURN_VOID_ON_FALSE ((rc = ble_gap_adv_rsp_set_fields (&rspFields)) == 0, ESP_FAIL, tag, "failed to set scan response data, rc = %d", rc);
    ESP_LOGI (tag, "advInit: scan response data set");

    // Set non-connectable and general discoverable mode to be a beacon
    ble_gap_adv_params advParams;
    memset (&advParams, 0, sizeof(advParams));
    
    advParams.conn_mode = BLE_GAP_CONN_MODE_UND;
    advParams.disc_mode = BLE_GAP_DISC_MODE_GEN;

    // Start advertising
    ESP_RETURN_VOID_ON_FALSE ((rc = ble_gap_adv_start (ownAddrType, nullptr, BLE_HS_FOREVER, &advParams, gapHandler, nullptr)) == 0, ESP_FAIL, tag, "failed to start advertising, rc = %d", rc);
    ESP_LOGI (tag, "advInit: advertising started");
}

// ===============================================================================
static int gapHandler (struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        // A new connection was established or a connection attempt failed.
        ESP_LOGI (tag, "gapHandler: BLE_GAP_EVENT_CONNECT, status = %d", event->connect.status);

        if (event->connect.status == 0) {
            ESP_RETURN_ON_FALSE ((rc = ble_gap_conn_find (event->connect.conn_handle, &desc)) == 0, rc, tag, "failed to find connection, rc = %d", rc);
            printConnDesc (&desc);
        }
        else {
            // Connection failed, resume advertising.
            advInit();
        }
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI (tag, "gapHandler: BLE_GAP_EVENT_DISCONNECT, reason = %d", event->disconnect.reason);
        printConnDesc (&event->disconnect.conn);

        // Connection terminated; resume advertising.
        advInit();
        break;

    case BLE_GAP_EVENT_CONN_UPDATE:
        // The central has updated the connection parameters.
        ESP_LOGI (tag, "gapHandler: BLE_GAP_EVENT_CONN_UPDATE, status = %d", event->conn_update.status);

        ESP_RETURN_ON_FALSE ((rc = ble_gap_conn_find (event->connect.conn_handle, &desc)) == 0, rc, tag, "failed to find connection, rc = %d", rc);
        printConnDesc (&desc);
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI (tag, "gapHandler: BLE_GAP_EVENT_ADV_COMPLETE, reason = %d", event->adv_complete.reason);
        
        advInit();
        break;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI (tag, "gapHandler: BLE_GAP_EVENT_MTU, conn_handle = %d, cid = %d, mtu = %d", event->mtu.conn_handle, event->mtu.channel_id, event->mtu.value);
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI (tag, "gapHandler: BLE_GAP_EVENT_SUBSCRIBE, conn_handle = %d, attr_handle = %d, reason = %d, prevn = %d, curn = %d, previ = %d, curi = %d"
            , event->subscribe.conn_handle
            , event->subscribe.attr_handle
            , event->subscribe.reason
            , event->subscribe.prev_notify
            , event->subscribe.cur_notify
            , event->subscribe.prev_indicate
            , event->subscribe.cur_indicate
        );
        //conn_handle_subs[event->subscribe.conn_handle] = true;
        break;

    default:
        ESP_LOGW (tag, "gapHandler: Unhandled GAP event: %d", event->type);
    }
    return 0;
}

// ===============================================================================
// It's called when host resets BLE stack due to errors
static void onStackReset(int reason) 
{
    ESP_LOGI(tag, "onStackReset: reset reason: %d", reason);
}

// ===============================================================================
// It's called when host has synced with controller
static void onStackSync() 
{
    // do advertising initialization
    ESP_LOGI(tag, "onStackSync: BLE stack is in sync, proceed to advInit");
    advInit();
}

// ===============================================================================
static void onGattSvrRegister (struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op) {
    case BLE_GATT_REGISTER_OP_SVC:
        ESP_LOGI (tag, "onGattSvrRegister: Registered service %s with handle = %d"
            , ble_uuid_to_str (ctxt->svc.svc_def->uuid, buf)
            , ctxt->svc.handle
        );
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        ESP_LOGI (tag, "onGattSvrRegister: Registered characteristic %s with def_handle = %d, val_handle = %d"
            , ble_uuid_to_str (ctxt->chr.chr_def->uuid, buf)
            , ctxt->chr.def_handle
            , ctxt->chr.val_handle
        );
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        ESP_LOGI (tag, "onGattSvrRegister: Registered descriptor %s with handle = %d"
            , ble_uuid_to_str (ctxt->dsc.dsc_def->uuid, buf)
            , ctxt->dsc.handle);
        break;

    default:
        ESP_LOGE (tag, "onGattSvrRegister: Unknown registration op: %d", ctxt->op);
        assert(0);
        break;
    }
}

// ===============================================================================
static esp_err_t nimbleInit () 
{
    ESP_RETURN_ON_ERROR (nimble_port_init(), tag, "nimble port init failed");

    ESP_LOGI (tag, "nimbleInit: nimble_port_init done, proceed to gapGattInit");

    ESP_RETURN_ON_ERROR (gapGattInit(), tag, "gap service init failed");

    ESP_LOGI (tag, "nimbleInit: gapGattInit done, proceed to set host callbacks");


    // Set host callbacks
    ble_hs_cfg.reset_cb = onStackReset;
    ble_hs_cfg.sync_cb = onStackSync;
    ble_hs_cfg.gatts_register_cb = onGattSvrRegister;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.sm_sc = 0;

    // Store host configuration
    ble_store_config_init();

    return ESP_OK;
}

// ===============================================================================
static void nimbleHostTask(void *param) 
{
    ESP_LOGI (tag, "nimbleHostTask: starting nimble_port_run");

    nimble_port_run();
    
    ESP_LOGI (tag, "nimbleHostTask: nimble_port_run returned, deinitializing nimble port");
    
    nimble_port_freertos_deinit();
}

// ===============================================================================
void testBle() 
{
    ESP_RETURN_VOID_ON_ERROR(nimbleInit(), tag, "nimble stack init failed");
    
    ESP_LOGI (tag, "testBle: nimble stack initialized, running main loop");

    nimble_port_freertos_init (nimbleHostTask);
}
