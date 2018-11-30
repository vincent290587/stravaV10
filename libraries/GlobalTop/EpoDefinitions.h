/*
 * EpoDefinitions.h
 *
 *  Created on: 28 janv. 2018
 *      Author: Vincent
 */

#ifndef LIBRARIES_EPODEFINITIONS_H_
#define LIBRARIES_EPODEFINITIONS_H_

#define EPO_SAT_SEGMENTS_NUM        (32*4)
#define EPO_SAT_SEGMENTS_NB         32
#define EPO_SAT_DATA_SIZE_BYTES     72
#define EPO_SAT_DATA_SIZE_WORDS     (EPO_SAT_DATA_SIZE_BYTES / 4)

#define MTK_FMT_NMEA_CMD_ID       0x00FD
#define MTK_EPO_BIN_CMD_ID        0x02D2
#define MTK_EPO_BIN_ACK_CMD_ID    0x0002
#define MTK_EPO_MAX_SAT_DATA      3

typedef struct {
	uint8_t sat_number;
	uint8_t sat[EPO_SAT_DATA_SIZE_BYTES];
} sEpoPacketSatData;

typedef struct {
	uint8_t nb_sat;
	uint16_t epo_seq;
	sEpoPacketSatData sat_data[MTK_EPO_MAX_SAT_DATA];
} sEpoPacketBinData;


#endif /* LIBRARIES_GLOBALTOP_EPODEFINITIONS_H_ */
