/*
 * time_sync.c
 *
 *  Created on: 27 февр. 2026 г.
 *      Author: Maksim
 
 TODO: подключить SNTP, получить системное время utc, вычислить offset
 */


#include "time_sync.h"
#include "monotonic.h"

#include "esp_log.h"
#include "esp_sntp.h"
#include <sys/time.h>
#include <time.h>

static const char *TAG = "TIME";

static int64_t s_time_offset_ms = 0;
static bool s_time_synced = false;

/* ---------- SNTP callback ---------- */

static void sntp_sync_cb(struct timeval *tv)
{	
	ESP_LOGW(TAG, "SNTP status: %d", esp_sntp_enabled());
    ESP_LOGW(TAG, "SNTP sync event received");
    
    char strftime_buf[64];
	struct tm timeinfo;

    time_t utc_s = (uint64_t)tv->tv_sec;
    uint64_t utc_ms = (uint64_t)tv->tv_usec / 1000ULL;
    setenv("TZ", "MSK-3", 1);
	tzset();
    localtime_r(&utc_s, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGW(TAG, "The current date/time in Moscow is: %s %d ms", strftime_buf, utc_ms);
    
    // Получаем UTC в миллисекундах
	utc_ms = ((uint64_t)tv->tv_sec * 1000ULL) +
             ((uint64_t)tv->tv_usec / 1000ULL);
    
    uint64_t mono_ms = monotonic_time_ms();

    s_time_offset_ms = (int64_t)utc_ms - (int64_t)mono_ms;

    s_time_synced = true;

    ESP_LOGW(TAG, "Time synced. Offset = %lld ms", s_time_offset_ms);
    ESP_LOGW(TAG, "UTC = %lld ms", utc_ms);
}

/* ---------- Init SNTP ---------- */

void time_sync_init(void)
{
    ESP_LOGW(TAG, "Initializing SNTP");

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);

    esp_sntp_setservername(0, "pool.ntp.org");

    esp_sntp_set_time_sync_notification_cb(sntp_sync_cb);

    esp_sntp_init();
    
    ESP_LOGW(TAG, "SNTP status: %d", esp_sntp_enabled());
}

/* ---------- API ---------- */

bool time_is_synced(void)
{
    return s_time_synced;
}

int64_t time_get_offset_ms(void)
{
    return s_time_offset_ms;
}

uint64_t time_get_utc_ms(void)
{
    if (!s_time_synced)
        return 0;

    return monotonic_time_ms() + s_time_offset_ms;
}



/*
#include "monotonic.h"
#include "time_sync.h"

static int64_t s_time_offset_ms = 0;
static bool s_time_synced = false;


void time_sync_set_offset(int64_t offset_ms)
{
    s_time_offset_ms = offset_ms;
    s_time_synced = true;
}

bool time_is_synced(void)
{
    return s_time_synced;
}

uint64_t time_get_utc_ms(void)
{
    return monotonic_time_ms() + s_time_offset_ms;
}
*/
