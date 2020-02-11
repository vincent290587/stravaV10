/*
 * GUI_connector.cpp
 *
 *  Created on: 6 d�c. 2018
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef WIN32
# include <winsock2.h>

#define SO_REUSEPORT      0
#define TCP_BUF_CAST      const char *
#define TCP_OPT_CAST      char *
#define TCP_OPT_TYPE      char
typedef int               socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOCKET            int
#define TCP_OPT_TYPE      int
#define TCP_BUF_CAST      void *
#define TCP_OPT_CAST      void *
#define INVALID_SOCKET    -1
#define WSACleanup()      do {} while (0)
#endif

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

SOCKET sockfd           = INVALID_SOCKET;
static SOCKET server_fd = INVALID_SOCKET;

static struct sockaddr_in address;
static int addrlen;
#endif

void GUI_connector_init(void) {

#ifdef LS027_GUI

	LOG_INFO("GUI Init");

	int valread;
	TCP_OPT_TYPE opt = 1;

	m_last_updated = millis();

	addrlen = sizeof(address);

#ifdef WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) /* Load Winsock 2.0 DLL */
	{
		perror("WSAStartup() failed");
		WSACleanup();
		exit(EXIT_FAILURE);
	}
#endif

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		perror("socket failed");
		WSACleanup();
		exit(EXIT_FAILURE);
	}

#ifdef WIN32
	//-------------------------
	// Set the socket I/O mode: In this case FIONBIO
	// enables or disables the blocking mode for the
	// socket based on the numerical value of iMode.
	// If iMode = 0, blocking is enabled;
	// If iMode != 0, non-blocking mode is enabled.
	int iResult;
	u_long iMode = 1;
	iResult = ioctlsocket(server_fd, FIONBIO, &iMode);
	if (iResult != NO_ERROR)
	  printf("ioctlsocket failed with error: %d\n", iResult);
#endif

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
			&opt, sizeof(opt)))
	{
		perror("setsockopt");
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons( PORT );

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("bind failed");
#ifdef WIN32
		closesocket(server_fd);
#else
		close(server_fd);
#endif
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
#ifdef WIN32
		closesocket(server_fd);
#else
		close(server_fd);
#endif
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	m_is_init = true;
#endif
}

void GUI_UpdateLS027(void) {

#ifdef LS027_GUI
	if (!m_is_init) return;

	memcpy(m_gui_comm.ls027_buffer, ls027_tdd_buffer, sizeof(ls027_tdd_buffer));


	if (sockfd != INVALID_SOCKET) {
		send(sockfd, (TCP_BUF_CAST)&m_gui_comm, sizeof(m_gui_comm), 0);
	} else {
		sockfd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
	}

#endif
}

extern "C" void neopixel_update(uint8_t red, uint8_t green, uint8_t blue) {

#ifdef LS027_GUI
	if (!m_is_init) return;

	if (millis() - m_last_updated < 250) return;

	m_last_updated = millis();

	m_gui_comm.neopixel[0] = red;
	m_gui_comm.neopixel[1] = green;
	m_gui_comm.neopixel[2] = blue;


	if (sockfd != INVALID_SOCKET) {
		send(sockfd, (TCP_BUF_CAST)&m_gui_comm, sizeof(m_gui_comm), 0);
	} else {
		sockfd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
	}

#endif
}

