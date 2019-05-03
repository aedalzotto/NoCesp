#include <stdio.h>

#include <comm.h>
#include <router.h>
#include <esp_console.h>
#include <esp_log.h>
#include <stdlib.h>
#include <string.h>
#include <driver/uart.h>
#include <esp_err.h>
#include <esp_vfs_dev.h>

#include "linenoise/linenoise.h"

static void conf_console();
static int route_cmd(int argc, char *argv[]);
static QueueHandle_t queue_ptr = NULL;

void comm_task(void *queue)
{
	queue_ptr = (QueueHandle_t)queue;
	conf_console();
	const char* prompt = LOG_COLOR_I "esp32> " LOG_RESET_COLOR;
	int probe_status = linenoiseProbe();
	if (probe_status) { /* zero indicates success */
        linenoiseSetDumbMode(1);
        /* Since the terminal doesn't support escape sequences,
         * don't use color codes in the prompt.
         */
        prompt = "esp32> ";
    }


	while(true){
		char* line = linenoise(prompt);
        if (line == NULL) { /* Ignore empty lines */
            continue;
        }
		/* Add the command to the history */
        linenoiseHistoryAdd(line);
		/* Try to run the command */
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND) {
            printf("Unrecognized command\n");
        } else if (err == ESP_ERR_INVALID_ARG) {
            // command was empty
        } else if (err == ESP_OK && ret != ESP_OK) {
            printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(err));
        } else if (err != ESP_OK) {
            printf("Internal error: %s\n", esp_err_to_name(err));
        }
        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
	}
}

static void conf_console()
{
	/* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    /* Configure UART. Note that REF_TICK is used so that the baud rate remains
     * correct while APB frequency is changing in light sleep mode.
     */
    const uart_config_t uart_config = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .use_ref_tick = true
    };
    ESP_ERROR_CHECK( uart_param_config(CONFIG_CONSOLE_UART_NUM, &uart_config) );

    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(CONFIG_CONSOLE_UART_NUM,
            256, 0, 0, NULL, 0) );

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONFIG_CONSOLE_UART_NUM);

    /* Initialize the console */
    esp_console_config_t console_config = {
            .max_cmdline_args = 8,
            .max_cmdline_length = 256,
            .hint_color = atoi(LOG_COLOR_CYAN)
    };
    ESP_ERROR_CHECK( esp_console_init(&console_config) );

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(100);

	esp_console_register_help_command();
	const esp_console_cmd_t cmd = {
        .command = "route",
        .help = "Route the leds attached to ESP",
        .hint = NULL,
        .func = &route_cmd,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

static int route_cmd(int argc, char *argv[])
{
	if (argc < 4) {
        printf("Número de argumentos inválido.\n");
        return 1;
    }

	bool topo;

	if(!strcmp(argv[1], "ring")){
		topo = false;
	} else if(!strcmp(argv[1], "mesh")){
		topo = true;
	} else {
		printf("Topologia inválida\n");
		return 1;
	}

	noc_node_t tmp = {};
	tmp.xsrc = atoi(argv[2]);
	if(!topo && argc == 4){
		tmp.xdst = atoi(argv[3]);
	} else if(topo && argc == 6){
		tmp.ysrc = atoi(argv[3]);
		tmp.xdst = atoi(argv[4]);
		tmp.ydst = atoi(argv[5]);
	} else {
		printf("Número de argumentos inválido.\n");
		return 1;
	}

	if(tmp.xsrc > 4 || tmp.ysrc > 4 || tmp.xdst > 4 || tmp.ydst > 4){
		printf("Tamanho inválido!\n");
		return 1;
	}

	xQueueSend(queue_ptr, &tmp, portMAX_DELAY);
	return 0;

}