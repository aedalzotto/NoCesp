#include <stdio.h>

#include <comm.h>
#include <router.h>


void comm_task(void *queue)
{
	//Init UART is needed?
	while(true){
		node_t tmp = {};
		printf("Enter with SOURCE(X, Y) DEST(X, Y): ");
		scanf("%su %su %su %su", &tmp.xsrc, &tmp.ysrc, &tmp.xdst, &tmp.ydst);

		if(tmp.xsrc > 4 || tmp.ysrc > 4 || tmp.xdst > 4 || tmp.ydst > 4){
			printf("Tamanho inválido!\n");
			continue;
		}

		xQueueSend((QueueHandle_t)queue, &tmp, portMAX_DELAY);
	}
}