/*
 * ble_buffer.h
 *
 *  Created on: 14 мар. 2026 г.
 *      Author: Maksim
 */

#ifndef MAIN_BLE_BUFFER_H_
#define MAIN_BLE_BUFFER_H_

#include <stdbool.h>
#include <stdint.h>

void ble_buffer_init(void);
bool ble_buffer_push(uint8_t *data, uint16_t len);
bool ble_buffer_pop(uint8_t *out, uint16_t *len);
bool ble_buffer_is_empty(void);
void ble_buffer_flush(void);


#endif /* MAIN_BLE_BUFFER_H_ */
