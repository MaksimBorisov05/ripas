/*
 * eth.c
 *
 *  Created on: 28 янв. 2026 г.
 *      Author: Maksim
 * Описание настройки и включения ethernet
 */
//-------------------ИСПРАВЛЕНИЯ-------------------

#include "eth.h"
#include "time_sync.h"

#include "esp_eth.h"    // Ethernet-драйвер
#include "esp_netif.h"  // TCP/IP интерфейс
#include "esp_event.h"  // События ETH
#include "esp_log.h"    // Логирование


#include "lwip/ip4_addr.h"    // Для назначения IP, gateway, mask


static const char *TAG = "ETH";


static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    if (event_base == ETH_EVENT) {
        switch (event_id) {
        case ETHERNET_EVENT_CONNECTED:
            ESP_LOGW("ETH", "Ethernet Connected");
            break;
        case ETHERNET_EVENT_DISCONNECTED:
            ESP_LOGW("ETH", "Ethernet Disconnected");
            break;
        case ETHERNET_EVENT_START:
            ESP_LOGW("ETH", "Ethernet Started");
            break;
        case ETHERNET_EVENT_STOP:
            ESP_LOGW("ETH", "Ethernet Stopped");
            break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_ETH_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGW("ETH", "Got IP Address");
        ESP_LOGW("ETH", "IP: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGW("ETH", "MASK: " IPSTR, IP2STR(&event->ip_info.netmask));
        ESP_LOGW("ETH", "GW: " IPSTR, IP2STR(&event->ip_info.gw));
        
        time_sync_init();
    }
}


void eth_init(void)
{
    ESP_LOGW(TAG, "Initializing Ethernet...");

    // 1. Netif + event loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *netif = esp_netif_new(&cfg);
    
    // Останавливаем DHCP тобы установить статический IP
    //ESP_ERROR_CHECK(esp_netif_dhcpc_stop(netif));
    // Назначаем статический IP
    //esp_netif_ip_info_t ip_info;
	//IP4_ADDR(&ip_info.ip, 192, 168, 1, 10);
	//IP4_ADDR(&ip_info.gw, 192, 168, 1, 1);
	//IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
	
	//ESP_ERROR_CHECK(esp_netif_set_ip_info(netif, &ip_info));

    // 2. MAC config
    // Рабочая конфигурация для WT32-ETH01 v1.4
    // MDC = 23, MDIO = 18, ClockMode = EMAC_CLK_EXT_IN, Clock GPIO = 0
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();

    eth_esp32_emac_config_t emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();
    emac_config.smi_gpio.mdc_num = 23;
    emac_config.smi_gpio.mdio_num = 18;

    // ВАЖНО: внешний RMII clock с LAN8720
    emac_config.clock_config.rmii.clock_mode = EMAC_CLK_EXT_IN;
	emac_config.clock_config.rmii.clock_gpio = 0;	//0 или 16 или 17


    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&emac_config, &mac_config);

    // 3. PHY config
    // Рабочая конфигурация для WT32-ETH01 v1.4
    // PHY Address = 1, Reset GPIO Number = 16
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = 1;         // 0 или 1
    phy_config.reset_gpio_num = 16;  // -1 или 16

    esp_eth_phy_t *phy = esp_eth_phy_new_lan87xx(&phy_config);

    // 4. Driver install
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle;
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));

    // 5. Attach netif
    esp_eth_netif_glue_handle_t glue = esp_eth_new_netif_glue(eth_handle);
    ESP_ERROR_CHECK(esp_netif_attach(netif, glue));
    ESP_ERROR_CHECK(esp_netif_dhcpc_start(netif)); // Запускает DHCP


    // 6. Event handlers
    esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, eth_event_handler, NULL);
	esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, eth_event_handler, NULL);

    // 7. Start Ethernet
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
}
