#include <commwifi.h>
#include <stdbool.h>

#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include <string.h>
#include <esp_log.h>
#include <router.h>

#define PORT_NUMBER 8888
#define BUF_SIZE (1024)

#define DEFAULT_SSID "SSID"
#define DEFAULT_PWD "PW"

#define TCPSERVER "IP.IP.IP.IP"

#define MESSAGE "sendMeCommand"

static const char *TAG = "WIFI";

static esp_err_t event_handler(void *ctx, system_event_t *event){
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
      ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START\n");
      ESP_ERROR_CHECK(esp_wifi_connect());
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP\n");
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED: ");
      ESP_ERROR_CHECK(esp_wifi_connect());
      break;
    default:
		break;
    }
    return ESP_OK;
}

void initialise_wifi(void){
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
	wifi_config_t wifi_config = {
        .sta = {
            .ssid = DEFAULT_SSID,
            .password = DEFAULT_PWD,
        },
    };
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_LOGI(TAG, "WiFi configurado\n");
}

void client_task(void *queue){
    ESP_LOGI(TAG,"tcp_client task started \n");
    struct sockaddr_in tcpServerAddr;
    tcpServerAddr.sin_addr.s_addr = inet_addr(TCPSERVER);
    tcpServerAddr.sin_family = AF_INET;
    tcpServerAddr.sin_port = htons(PORT_NUMBER);
    int s, r;
    char recv_buf[256];
    while(1){
		int mesh = 0;
        s = socket(AF_INET, SOCK_STREAM, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.\n");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket\n");
		if(connect(s, (struct sockaddr *)&tcpServerAddr, sizeof(tcpServerAddr)) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d \n", errno);
            close(s);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        
		r = read(s, recv_buf, sizeof(recv_buf)-1);
		if(recv_buf[0]=='m')
			mesh = true;
		else
			mesh = false;

		int step = mesh ? 4 : 2;

		r = read(s, recv_buf, sizeof(recv_buf)-1);
		for(int i = 0; i < r; i+=step){
			noc_node_t tmp = {};
			tmp.xsrc = recv_buf[i]-48;
			if(!mesh){
				tmp.xdst = recv_buf[i+1]-48;
			} else if(mesh){
				tmp.ysrc = recv_buf[i+1]-48;
				tmp.xdst = recv_buf[i+2]-48;
				tmp.ydst = recv_buf[i+3]-48;
			} else {
				ESP_LOGE(TAG, "Número de argumentos inválido.");
				break;
			}
			xQueueSend(queue, &tmp, portMAX_DELAY);
		}

		r = read(s, recv_buf, sizeof(recv_buf)-1);

        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
        close(s);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "...tcp_client task closed\n");
}