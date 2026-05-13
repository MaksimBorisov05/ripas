/*
 * protobuf_builder.h
 *
 *  Created on: 28 мар. 2026 г.
 *      Author: Maksim
 */

#ifndef MAIN_PROTOBUF_BUILDER_H_
#define MAIN_PROTOBUF_BUILDER_H_


#include "app_events.h"

#include <stdint.h>

uint16_t ble_protobuf_build(uint8_t *buf, const app_event_t *ev);


#endif /* MAIN_PROTOBUF_BUILDER_H_ */
