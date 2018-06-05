/*
 * MTK.h
 *
 *  Created on: 28 janv. 2018
 *      Author: Vincent
 */

#ifndef LIBRARIES_GLOBALTOP_MTK_H_
#define LIBRARIES_GLOBALTOP_MTK_H_

#include <stdint.h>
#include "EpoDefinitions.h"

#define MTK_DATA_START             6
#define MTK_MAX_PACKET_LENGTH      256



typedef struct {
	uint8_t preamble[2];
	uint8_t length[2];
	uint8_t command_id[2];
	uint8_t data[MTK_MAX_PACKET_LENGTH - 9];
	uint8_t checksum;
	uint8_t end_word[2];
} sMTKRawpacket ;

typedef struct {
	uint8_t  is_valid;
	uint16_t byte_count;
	uint16_t data_count;
	union {
		sMTKRawpacket raw_packet;
		uint8_t       raw_data[MTK_MAX_PACKET_LENGTH];
	};
} sMTKpacket;

#if defined(__cplusplus)

/**
 * wrapper for MTK packet management
 */
class MTK {
public:
	MTK(void);
	MTK(uint16_t command_id, uint8_t *buffer, uint16_t length);
	MTK(sEpoPacketBinData* epo);

	void clear(void);

	void fillPacket(uint16_t command_id, uint8_t *buffer, uint16_t length);

	uint16_t getPacketLength(void);
	uint16_t getDataLength(void);
	uint16_t getCommandId(void);

	uint8_t calculateChecksum(void);

	void toBuffer(uint8_t *buffer, uint16_t max_size);

	uint8_t encode(char c);
	uint8_t isValid(void);

	sMTKpacket m_packet;
private:

	void updateChecksum(void);
};

#endif /* _cplusplus */
#endif /* LIBRARIES_GLOBALTOP_MTK_H_ */
