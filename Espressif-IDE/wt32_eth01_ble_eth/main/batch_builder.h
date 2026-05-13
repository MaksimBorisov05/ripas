/*
 * batch_builder.h
 *
 *  Created on: 6 апр. 2026 г.
 *      Author: Maksim
 */

#ifndef MAIN_BATCH_BUILDER_H_
#define MAIN_BATCH_BUILDER_H_


#include <stdint.h>
#include <stdbool.h>
#include "app_events.h"

void batch_init(void);
bool batch_add_event(const app_event_t *ev);
uint16_t batch_build(uint8_t *buf);
bool batch_should_send(void);
void batch_reset(void);


#endif /* MAIN_BATCH_BUILDER_H_ */
