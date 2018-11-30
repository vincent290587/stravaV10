/*
 * EPONmeaPacket.h
 *
 *  Created on: 9 oct. 2018
 *      Author: Vincent
 */

#ifndef LIBRARIES_GLOBALTOP_EPONMEAPACKET_H_
#define LIBRARIES_GLOBALTOP_EPONMEAPACKET_H_

#include "EpoDefinitions.h"


int utc_to_gps_hour(int iYr, int iMo, int iDay, int iHr);

uint16_t antenova_epo_packet(sEpoPacketSatData *epo_data_, uint8_t *buffer_, uint16_t max_size);


#endif /* LIBRARIES_GLOBALTOP_EPONMEAPACKET_H_ */
