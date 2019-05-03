#include <router.h>
#include <freertos/task.h>

static void init_leds();
static void route(uint8_t xsrc, uint8_t ysrc, uint8_t xdst, uint8_t ydst);

static const gpio_num_t LEDS[4][4] = {{GPIO_NUM_32, GPIO_NUM_33, GPIO_NUM_25, GPIO_NUM_26},
									  {GPIO_NUM_27, GPIO_NUM_14, GPIO_NUM_12, GPIO_NUM_13},
									  {GPIO_NUM_23, GPIO_NUM_22, GPIO_NUM_21, GPIO_NUM_19},
									  {GPIO_NUM_18, GPIO_NUM_17, GPIO_NUM_16, GPIO_NUM_15}};

void router_task(void *queue)
{
	init_leds();
	while(true){
		noc_node_t tmp = {};
		xQueueReceive((QueueHandle_t)queue, &tmp, portMAX_DELAY);
		route(tmp.xsrc, tmp.ysrc, tmp.xdst, tmp.ydst);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

static void init_leds()
{
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[LEDS[i][j]], PIN_FUNC_GPIO);

			gpio_set_direction(LEDS[i][j], GPIO_MODE_OUTPUT);
		}
	}
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

	gpio_set_level(LEDS[xidx][yidx], 1);
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	gpio_set_level(LEDS[xidx][yidx], 0);
	vTaskDelay(500 / portTICK_PERIOD_MS);

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