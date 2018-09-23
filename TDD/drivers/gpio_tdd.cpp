/*
 * gpio.c

 *
 *  Created on: 19 sept. 2018
 *      Author: Vincent
 */


#ifdef LS027_GUI
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#endif

#include "gpio.h"
#include "Model_tdd.h"
#include "segger_wrapper.h"

extern int sockfd;

void gpio_set(uint16_t gpio_nb_) {

}

uint8_t gpio_get(uint16_t gpio_nb_) {
	return 0;
}

void register_btn_press(uint8_t btn_index) {

	switch (btn_index)
	{
	case 100:
		vue.tasks(eButtonsEventLeft);
		break;
	case 101:
		vue.tasks(eButtonsEventCenter);
		break;
	case 102:
		vue.tasks(eButtonsEventRight);
		break;
	default:
		return; // no implementation needed
	}
}

void btn_task(void) {

#ifdef LS027_GUI
	uint8_t RecvBuffer[1];

	if (sockfd > 0) {
		if (recv(sockfd, RecvBuffer, sizeof(RecvBuffer), 0) < 0) {
			// fail
		} else {
			if (RecvBuffer[0]) LOG_INFO("TCP recv %u", RecvBuffer[0]);
			register_btn_press(RecvBuffer[0]);
		}
	}
#endif

}
