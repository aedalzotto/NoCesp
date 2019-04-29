#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <router.h>
#include <comm.h>

void app_main()
{
    static QueueHandle_t queue_routes = xQueueCreate(128, sizeof(node_t));

    xTaskCreate(comm_task, "Communication_task", 2048, queue_routes, 10, NULL);
    // Communication should put requests in a queue to routing
    xTaskCreate(router_task, "Router_task", 2048, queue_routes, 10, NULL);
}