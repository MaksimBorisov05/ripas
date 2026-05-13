/*
 * ble.h
 *
 *  Created on: 12 февр. 2026 г.
 *      Author: Maksim
 */

#ifndef MAIN_BLE_H_
#define MAIN_BLE_H_


#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void ble_start(QueueHandle_t app_queue);


#endif /* MAIN_BLE_H_ */
