#include <router.h>
#include <freertos/task.h>

static void init_leds();
static void route(uint8_t xsrc, uint8_t ysrc, uint8_t xdst, uint8_t ydst);

void router_task(void *queue)
{
	init_leds();
	while(true){
		node_t tmp = {};
		xQueueReceive((QueueHandle_t)queue, &tmp, portMAX_DELAY);
		route(tmp.xsrc, tmp.ysrc, tmp.xdst, tmp.ydst);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

static void init_leds()
{
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
			gpio_set_direction(LEDS[i][j], GPIO_MODE_OUTPUT);
		
}

static void route(uint8_t xsrc, uint8_t ysrc, uint8_t xdst, uint8_t ydst)
{
	uint8_t xidx = xsrc;
	uint8_t yidx = ysrc;
	
	while(xidx != xdst){
		gpio_set_level(LEDS[xidx][yidx], 1);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		gpio_set_level(LEDS[xidx][yidx], 0);
		vTaskDelay(500 / portTICK_PERIOD_MS);
		xidx = xidx < xdst ? xidx+1 : xidx -1;
	}
	while(yidx != ydst){
		gpio_set_level(LEDS[xidx][yidx], 1);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		gpio_set_level(LEDS[xidx][yidx], 0);
		vTaskDelay(500 / portTICK_PERIOD_MS);
		yidx = yidx < ydst ? yidx+1 : yidx -1;
	}

	// Blink quickly 3 times when routed
	for(int i = 0; i < 3; i++){
		gpio_set_level(LEDS[xsrc][ysrc], 1);
		gpio_set_level(LEDS[xdst][ydst], 1);
		vTaskDelay(500 / portTICK_PERIOD_MS);
		gpio_set_level(LEDS[xsrc][ysrc], 0);
		gpio_set_level(LEDS[xdst][ydst], 0);
		vTaskDelay(333 / portTICK_PERIOD_MS);
	}
}