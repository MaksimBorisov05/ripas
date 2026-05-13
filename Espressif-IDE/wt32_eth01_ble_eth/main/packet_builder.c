/*
 * packet_builder.c
 *
 *  Created on: 22 мар. 2026 г.
 *      Author: Maksim
 */

#include "packet_builder.h"

#include <string.h>


uint16_t ble_packet_build(uint8_t *buf, const app_event_t *ev){
	uint16_t offset = 2;
					
	// MAC
	memcpy(&buf[offset], ev->data.ble.mac, 6);
	offset += 6;
	
	// RSSI
	buf[offset] = (uint8_t)ev->data.ble.rssi;
	offset++;
	
	// ADV Lenght
	buf[offset] = ev->data.ble.adv_len;
	offset++;
	
	// ADV Data
	memcpy(&buf[offset], ev->data.ble.adv_data, ev->data.ble.adv_len);
	offset += ev->data.ble.adv_len;
	
	// Monotonic
	uint64_t mt = ev->data.ble.monotonic_ms;
    memcpy(&buf[offset], &mt, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    
    // UTC = monotonic_ms + offset_ms
    uint64_t utc = ev->data.ble.utc_ms;
    memcpy(&buf[offset], &utc, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    
    // Time Sync
    buf[offset] = ev->data.ble.time_sync;
    offset++;
    
    // Длина полезной нагрузки
    uint16_t payload_len = offset - 2;
    buf[0] = payload_len & 0xFF;
    buf[1] = (payload_len >> 8) & 0xFF;
    
    return offset;
}
