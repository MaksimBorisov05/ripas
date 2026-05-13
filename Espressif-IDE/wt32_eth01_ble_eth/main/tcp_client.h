/*
 * tcp_client.h
 *
 *  Created on: 6 февр. 2026 г.
 *      Author: Maksim
 */

#ifndef MAIN_TCP_CLIENT_H_
#define MAIN_TCP_CLIENT_H_


//#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void tcp_client_start(QueueHandle_t app_queue);


#endif /* MAIN_TCP_CLIENT_H_ */
