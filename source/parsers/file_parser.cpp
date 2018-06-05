/*
 * file_parser.cpp
 *
 *  Created on: 18 oct. 2017
 *      Author: Vincent
 */

#include <string.h>
#include "file_parser.h"
#include "utils.h"
#include "segger_wrapper.h"


/**
 * charge un unique point dans le fichier deja ouvert
 * @param buffer
 * @param mon_segment
 * @param time_start
 * @return
 */
int chargerPointSeg(char *buffer, Segment& mon_segment, float& time_start) {

	int isError = 0;
	float lon, lat, alt, rtime;
	float data[4];
	uint8_t pos = 0;
	const char *deli = " ; ";
	char *pch;

	lat = 0; lon = 0; alt = 0;
	isError = 0;

	if (!buffer) return 1;

	// on se met au bon endroit
	pch = strtok (buffer, deli);
	while (pch != NULL && pos < 4)
	{
		data[pos] = strtof(pch, 0);
		pch = strtok (NULL, deli);
		pos++;
	}

	isError = pos != 4;

	if (!isError) {

		lat   = data[0];
		lon   = data[1];
		rtime = data[2];
		alt   = data[3];

		if (mon_segment.longueur() > 0) {
			rtime -= time_start;
			mon_segment.ajouterPointFin(lat, lon, alt, rtime);
		}
		else {
			// on init la liste
			time_start = rtime;
			mon_segment.ajouterPointFin(lat, lon, alt, 0.);
		}

		return 0;

	}
	else {
		// echec
		return isError;
	}
}


/**
 * charge un unique point dans le fichier deja ouvert
 * @param buffer
 * @param mon_parcours
 * @return
 */
int chargerPointPar(char *buffer, Parcours& mon_parcours) {

	static int isError = 0;
	float lon, lat, ele;
	float data[4];
	const char *deli = " ";
	uint8_t pos = 0;
	char *pch;

	lat = 0.; lon = 0.;
	isError = 1;

	if (!buffer) return 1;

	if (!strstr(buffer, deli)) return 1;

	// on se met au bon endroit
	pch = strtok (buffer, deli);
	while (pch != NULL && pos < 3)
	{
		data[pos++] = strtof(pch, 0);
		pch = strtok (NULL, deli);
	}
	isError = (pos < 2);

	if (!isError) {

		lat = data[0];
		lon = data[1];

		if (pos > 2) ele = data[2];
		else         ele = 0.;

		mon_parcours.ajouterPointFin(lat, lon, ele);

		return 0;

	}
	else {
		// echec
		LOG_ERROR("Point loading failed\r\n");

		return isError;
	}
}

/**
 *
 * @param name
 * @param lat_
 * @param lon_
 * @return
 */
int parseSegmentName(const char *name, float *lat_, float *lon_) {

	if (!name) {
		return 1;
	}

	if (strstr(name, "$"))
		return 1;

	calculePos (name, lat_, lon_);

	return 0;
}
