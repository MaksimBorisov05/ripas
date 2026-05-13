/*
 * ble.c
 *
 *  Created on: 12 февр. 2026 г.
 *      Author: Maksim
 */


#include "ble.h"
#include "app_events.h"
#include "monotonic.h"
#include "time_sync.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#include "host/ble_hs.h"
#include "host/ble_gap.h"

static const char *TAG = "BLE";
static QueueHandle_t s_app_queue;

/* -------- GAP callback -------- */
static int gap_event(struct ble_gap_event *event, void *arg){
	switch(event->type){
		case BLE_GAP_EVENT_DISC:
			app_event_t ev = {0};
			ev.type = APP_EVENT_BLE_DATA;
			
			memcpy(ev.data.ble.mac, event->disc.addr.val, 6);
			ev.data.ble.rssi = event->disc.rssi;
			ev.data.ble.adv_len = event->disc.length_data;
			if (ev.data.ble.adv_len > BLE_ADV_DATA_MAX_LEN){
				ev.data.ble.adv_len = BLE_ADV_DATA_MAX_LEN;
			}
			memcpy(ev.data.ble.adv_data, event->disc.data, ev.data.ble.adv_len);
			ev.data.ble.monotonic_ms = monotonic_time_ms();
			ev.data.ble.utc_ms = time_get_utc_ms();
			ev.data.ble.time_sync = time_is_synced();
			
			xQueueSend(s_app_queue, &ev, 0);
			
			break;
		
		case BLE_GAP_EVENT_DISC_COMPLETE:
			ESP_LOGW(TAG, "Scan complete, restarting...");
			ble_gap_disc(0, BLE_HS_FOREVER, NULL, gap_event, NULL); //BLE_ADDR_RANDOM или PUBLIC не помню
			break;
		
		default:
			break;
	}
	
	return 0;
}


/* -------- Start scanning -------- */
static void ble_app_on_sync(void){
	ESP_LOGW(TAG, "BLE stack synced, starting scan");
	
	// Discovery parameters (Параметры обнаружения) 
	struct ble_gap_disc_params params = {0};
	params.passive = 1;            // If passive scan should be used
	params.itvl = 0x50;            // Scan interval in 0.625ms units (50 ms)
	params.window = 0x30;          // Scan window in 0.625ms units
	params.filter_duplicates = 0;  // If enable duplicates filtering, 0-disable
	
	// Performs the Limited or General Extended Discovery Procedures
	ble_gap_disc(0, BLE_HS_FOREVER, &params, gap_event, NULL);
}


/* -------- Host task -------- */
static void ble_host_task(void *params){
	nimble_port_run(); // This function will return only when nimble_port_stop() is executed
    nimble_port_freertos_deinit();
}


/* -------- Public init -------- */

void ble_start(QueueHandle_t app_queue)
{
    ESP_LOGW(TAG, "BLE start");

    s_app_queue = app_queue;

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    nimble_port_init();

    ble_hs_cfg.sync_cb = ble_app_on_sync;

    nimble_port_freertos_init(ble_host_task);

    ESP_LOGW(TAG, "NimBLE initialized");
}





