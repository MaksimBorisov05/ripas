/*
 * transport.h
 *
 *  Created on: 6 февр. 2026 г.
 *      Author: Maksim
 */

#ifndef MAIN_TRANSPORT_H_
#define MAIN_TRANSPORT_H_


//#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void transport_attach(int sock);
void transport_detach(void);

bool transport_is_connected(void);
int  transport_send(const uint8_t *data, size_t len);


#endif /* MAIN_TRANSPORT_H_ */
