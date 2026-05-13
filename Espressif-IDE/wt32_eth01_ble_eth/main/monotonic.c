/*
 * monotonic.c
 *
 *  Created on: 25 февр. 2026 г.
 *      Author: Maksim
 */


#include "monotonic.h"
#include "esp_timer.h"

uint64_t monotonic_time_ms(void)
{
    return (uint64_t)(esp_timer_get_time() / 1000ULL);
}

