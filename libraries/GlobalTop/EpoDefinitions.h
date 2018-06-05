/*
 * EpoDefinitions.h
 *
 *  Created on: 28 janv. 2018
 *      Author: Vincent
 */

#ifndef LIBRARIES_GLOBALTOP_EPODEFINITIONS_H_
#define LIBRARIES_GLOBALTOP_EPODEFINITIONS_H_

#define MTK_EMPTY_CMD_ID          0x0000
#define MTK_ACK_CMD_ID            0x0001
#define MTK_FMT_NMEA_CMD_ID       0x00FD
#define MTK_EPO_BIN_CMD_ID        0x02D2
#define MTK_EPO_BIN_ACK_CMD_ID    0x0002

#define MTK_EPO_MAX_SAT_DATA      3
#define MTK_EPO_SAT_DATA_SIZE     60

typedef struct {
	uint8_t sat[MTK_EPO_SAT_DATA_SIZE];
} sEpoPacketSatData;

typedef struct {
	uint8_t nb_sat;
	uint16_t epo_seq;
	sEpoPacketSatData sat_data[MTK_EPO_MAX_SAT_DATA];
} sEpoPacketBinData;


#endif /* LIBRARIES_GLOBALTOP_EPODEFINITIONS_H_ */
