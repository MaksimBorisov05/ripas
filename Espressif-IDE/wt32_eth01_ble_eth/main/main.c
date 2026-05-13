#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "eth.h"
#include "app.h"

void app_main(void)
{
    eth_init();
    app_start();
	
	/*
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }*/
}
