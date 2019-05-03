#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <stdint.h>
#include <driver/gpio.h>

typedef struct _noc_nodes {
	uint8_t xsrc;
	uint8_t ysrc;
	uint8_t xdst;
	uint8_t ydst;
} noc_node_t;

void router_task(void *queue);