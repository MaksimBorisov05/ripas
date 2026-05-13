/*
 * tcp_client.c
 *
 *  Created on: 6 февр. 2026 г.
 *      Author: Maksim
 * Поддержка TCP соединения
 */


#include "tcp_client.h"
#include "transport.h"
#include "app_events.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "esp_log.h"
#include "freertos/task.h"

static const char *TAG = "TCP";

//#define SERVER_IP   "192.168.1.1"	// Для Linux
#define SERVER_IP   "192.168.137.1"	// Для Windows
#define SERVER_PORT 9000

static QueueHandle_t s_app_queue;

static void tcp_client_task(void *arg)
{
    while (1) {
        struct sockaddr_in server_addr = {0};
		
		// Создаем сокет
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
        server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

        ESP_LOGW(TAG, "Connecting to server...");
        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
            ESP_LOGE(TAG, "Socket connect failed");
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        ESP_LOGI(TAG, "TCP connected");

        transport_attach(sock);

        app_event_t ev = { .type = APP_EVENT_TCP_CONNECTED };
        xQueueSend(s_app_queue, &ev, 0);

        while (transport_is_connected()) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        ESP_LOGW(TAG, "TCP disconnected");

        app_event_t ev2 = { .type = APP_EVENT_TCP_DISCONNECTED };
        xQueueSend(s_app_queue, &ev2, 0);

        transport_detach();
        close(sock);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void tcp_client_start(QueueHandle_t app_queue)
{
    s_app_queue = app_queue;
    xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
}