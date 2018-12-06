/*
 * GUI_connector.cpp
 *
 *  Created on: 6 déc. 2018
 *      Author: Vincent
 */

/*
 * ls027_tdd.c
 *
 *  Created on: 18 sept. 2018
 *      Author: Vincent
 */


#ifdef LS027_GUI
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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <millis.h>
#include <stdbool.h>
#include "ls027.h"
#include "GUI_connector.h"
#include "segger_wrapper.h"

typedef struct {
	uint8_t ls027_buffer[LS027_BUFFER_SIZE];
	uint8_t data_flags;
	uint8_t neopixel[3];
} sGUIComm;

#ifdef LS027_GUI

#define PORT 8080

extern uint8_t ls027_tdd_buffer[LS027_BUFFER_SIZE];

static uint32_t m_last_updated;

static sGUIComm m_gui_comm;

static bool m_is_init = false;

int sockfd = -1;
static int server_fd = -1;

struct sockaddr_in address;
static int addrlen;
#endif

void GUI_connector_init(void) {

#ifdef LS027_GUI

	LOG_INFO("GUI Init");

	int valread;
	int opt = 1;

	m_last_updated = millis();

	addrlen = sizeof(address);

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
			&opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address,
			sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	m_is_init = true;
#endif
}

void GUI_UpdateLS027(void) {

#ifdef LS027_GUI
	if (!m_is_init) return;

	memcpy(m_gui_comm.ls027_buffer, ls027_tdd_buffer, sizeof(ls027_tdd_buffer));


	if (sockfd > 0) {
		send(sockfd, &m_gui_comm, sizeof(m_gui_comm), 0);
	} else {
		sockfd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
	}


	LOG_INFO("LS027 Updated");
#endif
}

extern "C" void neopixel_update(uint8_t red, uint8_t green, uint8_t blue) {

#ifdef LS027_GUI
	if (!m_is_init) return;

	if (millis() - m_last_updated < 5) return;

	m_last_updated = millis();

	m_gui_comm.neopixel[0] = red;
	m_gui_comm.neopixel[1] = green;
	m_gui_comm.neopixel[2] = blue;


	if (sockfd > 0) {
		send(sockfd, &m_gui_comm, sizeof(m_gui_comm), 0);
	} else {
		sockfd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
	}

	LOG_INFO("Neopixel Updated");
#endif
}

