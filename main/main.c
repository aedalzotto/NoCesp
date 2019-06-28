#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <router.h>
#include <comm.h>
#include <commwifi.h>
#include <nvs_flash.h>

void app_main()
{
    static QueueHandle_t queue_routes = NULL;
    queue_routes = xQueueCreate(128, sizeof(noc_node_t));

    ESP_ERROR_CHECK(nvs_flash_init());
    initialise_wifi();

    xTaskCreate(client_task, "WiFi_task", 8192, queue_routes, 10, NULL);
    // xTaskCreate(comm_task, "Communication_task", 2048, queue_routes, 10, NULL);
    // Communication should put requests in a queue to routing
    xTaskCreate(router_task, "Router_task", 2048, queue_routes, 10, NULL);
}