/*
 * app_events.h
 *
 *  Created on: 8 февр. 2026 г.
 *      Author: Maksim
 * Описание собственных событий/состояний системы
 */

#ifndef MAIN_APP_EVENTS_H_
#define MAIN_APP_EVENTS_H_


//#pragma once
#include <stdint.h>
#include <stdbool.h>


#define BLE_ADV_DATA_MAX_LEN 31

typedef struct {
    uint8_t  mac[6];
    int8_t   rssi;
    uint8_t  adv_data[BLE_ADV_DATA_MAX_LEN];
    uint8_t  adv_len;
    uint64_t utc_ms; 	   // Реальное время utc (на будущее после синхронизации)
    uint64_t monotonic_ms; // Монотонное время, считается после запуска контроллера
    bool time_sync;        // Флаг синхронизации времени
} ble_event_t;

typedef enum {
    APP_EVENT_BLE_DATA,
    APP_EVENT_TCP_CONNECTED,
    APP_EVENT_TCP_DISCONNECTED,
} app_event_type_t;

typedef struct {
    app_event_type_t type;
    union {
		ble_event_t ble;
    } data;
} app_event_t;


#endif /* MAIN_APP_EVENTS_H_ */
