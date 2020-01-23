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
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

extern int sockfd;

#endif

#include "boards.h"
#include "gpio.h"
#include "Model.h"
#include "segger_wrapper.h"

static uint8_t m_gpios_states[0x3F + 1];
static uint8_t m_is_init = 0;

void gpio_set(uint16_t gpio_nb_) {

	if (!m_is_init) {
		m_is_init = 1;
		memset(m_gpios_states, 2, sizeof(m_gpios_states));
	}

	if (m_gpios_states[gpio_nb_ & 0x3F] == 0 || m_gpios_states[gpio_nb_ & 0x3F] == 2) {
		LOG_INFO("Setting GPIO %u-%u", gpio_nb_ >> 5, gpio_nb_ & 0x1F);
	}

	m_gpios_states[gpio_nb_ & 0x3F] = 1;

	if (gpio_nb_ == KILL_PIN) {
		exit(0);
	}
}

void gpio_clear(uint16_t gpio_nb_) {

	if (!m_is_init) {
		m_is_init = 1;
		memset(m_gpios_states, 2, sizeof(m_gpios_states));
	}

	if (m_gpios_states[gpio_nb_ & 0x3F] == 1 || m_gpios_states[gpio_nb_ & 0x3F] == 2) {
		LOG_INFO("Clearing GPIO %u-%u", gpio_nb_ >> 5, gpio_nb_ & 0x1F);
	}

	m_gpios_states[gpio_nb_ & 0x3F] = 0;
}

uint8_t gpio_get(uint16_t gpio_nb_) {

	if (!m_is_init) {
		m_is_init = 1;
		memset(m_gpios_states, 2, sizeof(m_gpios_states));
	}

	return m_gpios_states[gpio_nb_ & 0x3F] & 0x1;
}

void register_btn_press(uint8_t btn_index) {

	if (!btn_index) return;
	LOG_INFO("BTN press %u", btn_index);

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
