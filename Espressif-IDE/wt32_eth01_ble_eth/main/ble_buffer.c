/*
 * ble_buffer.c
 *
 *  Created on: 14 мар. 2026 г.
 *      Author: Maksim
 */

#include "ble_buffer.h"
#include "transport.h"

#include <string.h>
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define BLE_BUFFER_CAPACITY 128
#define BLE_PACKET_MAX_SIZE 640

static const char *TAG = "BUFFER";

typedef struct{
	uint8_t data[BLE_PACKET_MAX_SIZE];
	uint16_t len;
}ble_buffer_t;

static ble_buffer_t s_ringBuffer[BLE_BUFFER_CAPACITY];

static uint16_t s_head;
static uint16_t s_tail;
static uint16_t s_count;

static SemaphoreHandle_t s_buffer_mutex;


void ble_buffer_init(void){
	s_head = s_tail = s_count = 0;
	
	s_buffer_mutex = xSemaphoreCreateMutex();
	
	ESP_LOGW(TAG, "Ble buffer initialized");
}


bool ble_buffer_push(uint8_t *data, uint16_t len){
	if (xSemaphoreTake(s_buffer_mutex, portMAX_DELAY) != pdTRUE)
        return false;
        
	memcpy(s_ringBuffer[s_head].data, data, len);
	s_ringBuffer[s_head].len = len;
	s_head = (s_head+1) % BLE_BUFFER_CAPACITY;
	
	if(s_count==BLE_BUFFER_CAPACITY){
		xSemaphoreGive(s_buffer_mutex);
		return false;
	}
	
	s_count++;
	xSemaphoreGive(s_buffer_mutex);
	return true;
}


bool ble_buffer_pop(uint8_t *out, uint16_t *len){
	if (xSemaphoreTake(s_buffer_mutex, portMAX_DELAY) != pdTRUE)
        return false;
	
	if(s_count==0){
		xSemaphoreGive(s_buffer_mutex);
		return false;
	}
	
	if(s_count==BLE_BUFFER_CAPACITY){
		s_tail = s_head;
	}
	memcpy(out, s_ringBuffer[s_tail].data, s_ringBuffer[s_tail].len);
	*len = s_ringBuffer[s_tail].len;
	s_tail = (s_tail+1) % BLE_BUFFER_CAPACITY;
	s_count--;
	xSemaphoreGive(s_buffer_mutex);
	return true;
}


bool ble_buffer_is_empty(){
	return s_count == 0;
}


void ble_buffer_flush(void){
	ESP_LOGW(TAG, "Отправлено строк/сообщений: %d", s_count);
	
	uint8_t buf[BLE_PACKET_MAX_SIZE];
	uint16_t len;
	
	while(!ble_buffer_is_empty()){
		if(ble_buffer_pop(buf, &len)){
			transport_send(buf, len);
		}
	}
}

