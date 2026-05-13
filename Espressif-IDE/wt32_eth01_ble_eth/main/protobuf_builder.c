/*
 * protobuf_builder.c
 *
 *  Created on: 28 мар. 2026 г.
 *      Author: Maksim
 */

#include "protobuf_builder.h"

#include "scan.pb.h"
#include "pb_encode.h"

#include <string.h>

#include "esp_log.h"

static const char *TAG = "PROTO";


uint16_t ble_protobuf_build(uint8_t *buf, const app_event_t *ev){
	// 1. Создаём stream (пропускаем первые 2 байта под length)
	pb_ostream_t stream = pb_ostream_from_buffer(buf + 2, 256);
	// 2. Создаём protobuf сообщение
	ripas_BleEvent event_proto = ripas_BleEvent_init_zero;
	
	// 3. Заполнение полей
	// MAC
	event_proto.mac.size = 6;
	memcpy(event_proto.mac.bytes, ev->data.ble.mac, 6);
	
	// RSSI
	event_proto.rssi = ev->data.ble.rssi;
	
	// ADV Data and ADV Lenght
	uint8_t  adv_len = ev->data.ble.adv_len;
	event_proto.adv_data.size = adv_len;
	memcpy(event_proto.adv_data.bytes, ev->data.ble.adv_data, adv_len);
	
	// Monotonic
	event_proto.monotonic_ms = ev->data.ble.monotonic_ms;
    
    // UTC = monotonic_ms + offset_ms
    event_proto.utc_ms = ev->data.ble.utc_ms;
    
    // Time Sync
    event_proto.time_sync = ev->data.ble.time_sync;
	
	// 3. Encode
	if (!pb_encode(&stream, ripas_BleEvent_fields, &event_proto)) {
	    ESP_LOGE(TAG, "Encode failed");
	    return 0;
	}
	
	// 4. Длина payload
	uint16_t payload_len = stream.bytes_written;

    // 5. Записываем length header
    buf[0] = payload_len & 0xFF;
    buf[1] = (payload_len >> 8) & 0xFF;
    
    ESP_LOGI(TAG, "Encoded size: %d", payload_len);

    return payload_len + 2;
}


