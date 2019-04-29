#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <stdint.h>
#include <driver/gpio.h>

typedef struct _nodes {
	uint8_t xsrc;
	uint8_t ysrc;
	uint8_t xdst;
	uint8_t ydst;
} node_t;

static const gpio_num_t LEDS[4][4] = {{GPIO_NUM_32, GPIO_NUM_33, GPIO_NUM_25, GPIO_NUM_26},
									  {GPIO_NUM_27, GPIO_NUM_14, GPIO_NUM_12, GPIO_NUM_13},
									  {GPIO_NUM_23, GPIO_NUM_22, GPIO_NUM_21, GPIO_NUM_19},
									  {GPIO_NUM_18, GPIO_NUM_17, GPIO_NUM_16, GPIO_NUM_15}};

void router_task(void *queue);