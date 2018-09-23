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
#include "segger_wrapper.h"

#define PORT 8080

#define LS027_BUFFER_SIZE               ((LS027_DISPLAY_HW_NOF_ROWS * LS027_DISPLAY_HW_NOF_COLUMNS) / 8)

#define adagfxswap(a, b) { int16_t t = a; a = b; b = t; }

#define LS027_COORD_TO_INDEX(X,Y)       ((Y*LS027_HW_WIDTH + X) / 8)

static const uint8_t set[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
static const uint8_t clr[] = { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F };

static bool m_is_color_inverted = false;

static uint8_t ls027_tdd_buffer[LS027_BUFFER_SIZE];

#ifdef LS027_GUI
int sockfd = -1;
static int server_fd = -1;

struct sockaddr_in address;
static int addrlen;
#endif

/////////  STATIC FUNCTIONS

static void ls027_spi_buffer_clear(void) {
	LOG_DEBUG("LS027 buffers cleared");

	if (!m_is_color_inverted) {
		memset(ls027_tdd_buffer, LS027_PIXEL_GROUP_WHITE, sizeof(ls027_tdd_buffer));
	} else {
		memset(ls027_tdd_buffer, LS027_PIXEL_GROUP_BLACK, sizeof(ls027_tdd_buffer));
	}
}

/////////  FUNCTIONS

void LS027_Clear(void) {
	LOG_DEBUG("LS027 buffers cleared");

	if (!m_is_color_inverted) {
		memset(ls027_tdd_buffer, LS027_PIXEL_GROUP_WHITE, sizeof(ls027_tdd_buffer));
	} else {
		memset(ls027_tdd_buffer, LS027_PIXEL_GROUP_BLACK, sizeof(ls027_tdd_buffer));
	}
}

void LS027_Init(void) {

	LOG_INFO("LS027 Init");

#ifdef LS027_GUI

	int valread;
	int opt = 1;

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
#endif
}

void LS027_InvertColors(void) {
	m_is_color_inverted = !m_is_color_inverted;
}

void LS027_UpdateFull(void) {

#ifdef LS027_GUI
	if (sockfd > 0) {
		send(sockfd, ls027_tdd_buffer, sizeof(ls027_tdd_buffer), 0 );
	} else {
		sockfd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
	}
#endif

	ls027_spi_buffer_clear();

	LOG_INFO("LS027 Updated");
}

void LS027_ToggleVCOM(void) {

}

uint16_t LS027_get_pixel(uint16_t x, uint16_t y) {

	uint16_t ret_color = 0;

	ret_color = ls027_tdd_buffer[LS027_COORD_TO_INDEX(x,y)] & set[x & 7];

	return ret_color;
}

void LS027_drawPixel(uint16_t x, uint16_t y, uint16_t color) {

	bool _is_color_inverted = m_is_color_inverted;

	//we simply invert the pixel's color
	if (color == 2) {
		_is_color_inverted = false;
		color = LS027_get_pixel(x, y) ? 0:1;
	}

	// fill buffer
	if ((color && !_is_color_inverted) ||
			(!color && _is_color_inverted)) {
		ls027_tdd_buffer[LS027_COORD_TO_INDEX(x,y)] |= set[x & 7];
	} else {
		ls027_tdd_buffer[LS027_COORD_TO_INDEX(x,y)] &= clr[x & 7];
	}

}
