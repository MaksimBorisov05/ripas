/*
 * batch_builder.c
 *
 *  Created on: 6 апр. 2026 г.
 *      Author: Maksim
 */

#include "batch_builder.h"

#include "scan.pb.h"
#include "pb_encode.h"

#include <string.h>
#include "esp_timer.h"

#include "esp_log.h"

// Отправка по количеству 10 сообщений
#define MAX_BATCH_EVENTS 10
// Отправка по времени 100мс
#define BATCH_TIMEOUT_MS 100

static ripas_BleEvent s_events[MAX_BATCH_EVENTS];
static uint8_t s_count = 0;

static uint64_t s_last_flush_ms = 0;
static uint64_t s_batch_seq = 0;


void batch_init(void){
	s_count = 0;
	s_last_flush_ms = esp_timer_get_time()/1000;
}


bool batch_add_event(const app_event_t *ev){
	if(s_count >= MAX_BATCH_EVENTS){
		return false;
	}
	
    ripas_BleEvent *dst = &s_events[s_count];
    ripas_BleEvent tmp = ripas_BleEvent_init_zero;
    *dst = tmp;
	
	// MAC
	dst->mac.size = 6;
	memcpy(dst->mac.bytes, ev->data.ble.mac, 6);
	
	// RSSI
	dst->rssi = ev->data.ble.rssi;
	
	// ADV Data and ADV Lenght
	uint8_t adv_len = ev->data.ble.adv_len;
	dst->adv_data.size = adv_len;
	memcpy(dst->adv_data.bytes, ev->data.ble.adv_data, adv_len);
	
	// Monotonic
	dst->monotonic_ms = ev->data.ble.monotonic_ms;
	
	// UTC = monotonic_ms + offset_ms
	dst->utc_ms = ev->data.ble.utc_ms;
	
	// Time Sync
	dst->time_sync = ev->data.ble.time_sync;
	
	s_count++;
	return true;
}


bool batch_should_send(void){
	uint64_t now = esp_timer_get_time()/1000;
	
	if(s_count >= MAX_BATCH_EVENTS){
		return true;
	}
	
	if(s_count > 0 && (now - s_last_flush_ms >= BATCH_TIMEOUT_MS)){
		return true;
	}
	
	return false;
}


uint16_t batch_build(uint8_t *buf){
	// 1. Создаём stream (пропускаем первые 2 байта под length)
	pb_ostream_t stream = pb_ostream_from_buffer(buf + 2, 640);		// pb_ostream_t pb_ostream_from_buffer(pb_byte_t *buf, size_t bufsize);
	// 2. Создаём protobuf сообщение
	ripas_ScanBatch batch_proto = ripas_ScanBatch_init_zero;
	// Порядок (sequence) сообщений
	batch_proto.batch_seq = s_batch_seq++;
	// Repeated BleEvent
	batch_proto.ble_count = s_count;
	
	for (int i = 0; i < s_count; i++) {
        batch_proto.ble[i] = s_events[i];
    }

    if (!pb_encode(&stream, ripas_ScanBatch_fields, &batch_proto)) {
        return 0;
    }

    ESP_LOGI("BTCH BLDR", "Отправлено/записано BleEvents: %d", s_count);
    
    uint16_t payload_len = stream.bytes_written;

    buf[0] = payload_len & 0xFF;
    buf[1] = (payload_len >> 8) & 0xFF;

    return payload_len + 2;
}


void batch_reset(void){
    s_count = 0;
    s_last_flush_ms = esp_timer_get_time() / 1000;
}







