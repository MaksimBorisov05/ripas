/*
 * transport_tcp.c
 *
 *  Created on: 6 февр. 2026 г.
 *      Author: Maksim
 * Создание сокета TCP и отправка пакетов
 */


#include "transport.h"

#include <sys/socket.h>
#include <unistd.h>

#include "esp_log.h"

static const char *TAG = "TRANSPORT";
static int s_transport_sock = -1;

void transport_attach(int sock)
{
    s_transport_sock = sock;
    ESP_LOGI(TAG, "Transport attached");
}

void transport_detach(void)
{
    s_transport_sock = -1;
    ESP_LOGI(TAG, "Transport detached");
}

bool transport_is_connected(void)
{
    return s_transport_sock >= 0;
}

int transport_send(const uint8_t *data, size_t len)
{
    if (s_transport_sock < 0) {
        return -1;
    }

    int ret = send(s_transport_sock, data, len, 0);
    if (ret < 0) {
        ESP_LOGE(TAG, "send failed");
        s_transport_sock = -1;
    }
    return ret;
}

