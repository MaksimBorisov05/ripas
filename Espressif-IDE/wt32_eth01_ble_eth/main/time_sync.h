/*
 * time_sync.h
 *
 *  Created on: 27 февр. 2026 г.
 *      Author: Maksim
 */

#ifndef MAIN_TIME_SYNC_H_
#define MAIN_TIME_SYNC_H_


#include <stdint.h>
#include <stdbool.h>

//static void sntp_sync_time(struct timeval *tv)
void time_sync_init(void);
bool time_is_synced(void);
uint64_t time_get_utc_ms(void);
int64_t time_get_offset_ms(void);



/*
#include <stdbool.h>

void time_sync_set_offset(int64_t offset_ms);
bool time_is_synced(void);
uint64_t time_get_utc_ms(void);
*/

#endif /* MAIN_TIME_SYNC_H_ */
