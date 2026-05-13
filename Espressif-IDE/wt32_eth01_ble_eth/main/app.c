/*
 * app.c
 *
 *  Created on: 7 февр. 2026 г.
 *      Author: Maksim
 * Логика системы
 */


#include "app.h"
#include "app_events.h"
#include "tcp_client.h"
#include "transport.h"
#include "ble.h"
#include "ble_buffer.h"
#include "packet_builder.h"
#include "protobuf_builder.h"
#include "batch_builder.h"

#include "freertos/FreeRTOS.h"	// Базовые определения
#include "freertos/task.h"		// Задачи (xTaskCreate, vTaskDelay)
#include "freertos/queue.h"		// Очереди (xQueueCreate, xQueueSend, xQueueReceive)

#include "esp_log.h"			// Вывод логов в консоль


static const char *TAG = "APP";

static QueueHandle_t app_queue;

bool use_protobuf = true;		// Использование protobuf. Если true то protobuf_builder, если false то packet_builder


static void app_task(void *arg){
	app_event_t event;

	while(1){
		if(xQueueReceive(app_queue, &event, portMAX_DELAY)){
			
			switch(event.type){
				
				case APP_EVENT_BLE_DATA:
					ESP_LOGI(TAG, "Обнаружено BLE event rssi=%d", event.data.ble.rssi);
					// Здесь packet_builder + buffer (сборка пакета, помещение его в буфер, отправка при наличии TCP)
					batch_add_event(&event);

					if (batch_should_send()) {
					
					    uint8_t buffer[640];
					    uint16_t len = batch_build(buffer);
					
					    if (len > 0) {
					        if (transport_is_connected()) {
					            transport_send(buffer, len);
					        } else {
					            ble_buffer_push(buffer, len);
					        }
					    }
					
					    batch_reset();
					}
					
					
					
					//--------------------СТАРАЯ КОНСТРУКЦИЯ ДЛЯ PACKET И PROTO БЕЗ SCANBATCH--------------------(УДАЛИТЬ ПОСЛЕ ОТЛАДКИ SCANBATCH)
					/*
					//uint8_t buffer[2+6+1+1+BLE_ADV_DATA_MAX_LEN+8+8+1];	Используется для packet builder
					uint8_t buffer[256];									// Используется для protobuf builder
					uint16_t len;
					
					if(use_protobuf){
						len = ble_protobuf_build(buffer, &event);
					}
					else{
						len = ble_packet_build(buffer, &event);
					}
					
			        
					if(transport_is_connected()){
						transport_send(buffer, len);
					}
					
					else{
						if(!ble_buffer_push(buffer, len)){
							ESP_LOGW(TAG, "Буфер переполнен");
						}
					}
					*/
					//--------------------СТАРАЯ КОНСТРУКЦИЯ ДЛЯ PACKET И PROTO БЕЗ SCANBATCH--------------------(УДАЛИТЬ ПОСЛЕ ОТЛАДКИ SCANBATCH)
					break;
				
				case APP_EVENT_TCP_CONNECTED:
					// Здесь flush buffer (transport слой установил TCP соединение, теперь можно отправить всё накопленное)
					ble_buffer_flush();
					break;
				
				case APP_EVENT_TCP_DISCONNECTED:
					// Транспорт недоступен, данные нужно буферизовать или перейти в offline режим 
					ESP_LOGW(TAG, "Режим накапливания данных");
					break;
			}
		}
	}
}


void app_start(void){
	app_queue = xQueueCreate(32, sizeof(app_event_t));
	if(!app_queue){
		ESP_LOGE(TAG, "Failed to create app queue");
		return;
	}
	
	// Запуск BLE и TCP с передачей очереди
	tcp_client_start(app_queue);
	
	xTaskCreate(app_task, "app_task", 4096, NULL, 6, NULL);
	
	ble_start(app_queue);
	
	// Инициализация буфера	(сброс параметров)
	ble_buffer_init();
	
	// Инициализация компоновщика батчей
	batch_init();
}

