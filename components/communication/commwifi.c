#include <commwifi.h>

#include <stdbool.h>

void client_task(void *queue)
{
	char rx_buffer[256];
	char addr_str[128];
	int addr_family;
	int ip_protocol;

	ESP_ERROR_CHECK(nvs_flash_init());
	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	// TODO:
	// CONNECT

	while(true){
		struct sockaddr_in dest_addr;
		
	}
}