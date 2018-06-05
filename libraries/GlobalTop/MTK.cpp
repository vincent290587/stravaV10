/*
 * MTK.cpp
 *
 *  Created on: 28 janv. 2018
 *      Author: Vincent
 */

#include <MTK.h>
#include <string.h>
#include <utils.h>
#include "nrf_assert.h"
#include "segger_wrapper.h"

MTK::MTK(void) {

	memset(&m_packet, 0, sizeof(m_packet));

}

/**
 *
 * @param command_id
 * @param buffer
 * @param length
 */
MTK::MTK(uint16_t command_id, uint8_t* buffer, uint16_t length) : MTK() {

	if (buffer) {
		this->fillPacket(command_id, buffer, length);
	}
}

/**
 *
 * @param epo
 */
MTK::MTK(sEpoPacketBinData* epo) {

	ASSERT(epo);
	m_packet.raw_packet.preamble[0] = 0x04;
	m_packet.raw_packet.preamble[1] = 0x24;

	m_packet.data_count = 182;

	encode_uint16 (m_packet.raw_packet.command_id, MTK_EPO_BIN_CMD_ID);
	encode_uint16 (m_packet.raw_packet.length    , m_packet.data_count + 9);

	encode_uint16 (m_packet.raw_packet.data, epo->epo_seq);

	memcpy(m_packet.raw_packet.data+2  , epo->sat_data[0].sat, MTK_EPO_SAT_DATA_SIZE);
	memcpy(m_packet.raw_packet.data+62 , epo->sat_data[1].sat, MTK_EPO_SAT_DATA_SIZE);
	memcpy(m_packet.raw_packet.data+122, epo->sat_data[2].sat, MTK_EPO_SAT_DATA_SIZE);

	this->updateChecksum();

	m_packet.raw_packet.end_word[0] = 0x0D;
	m_packet.raw_packet.end_word[1] = 0x0A;

}

/**
 *
 */
void MTK::clear(void) {

	memset(&m_packet, 0, sizeof(m_packet));

}

/**
 *
 * @param command_id
 * @param buffer
 * @param length
 */
void MTK::fillPacket(uint16_t command_id, uint8_t* buffer, uint16_t length) {

	ASSERT(buffer);
	m_packet.raw_packet.preamble[0] = 0x04;
	m_packet.raw_packet.preamble[1] = 0x24;

	m_packet.data_count = length;

	encode_uint16 (m_packet.raw_packet.command_id, command_id);
	encode_uint16 (m_packet.raw_packet.length    , length + 9);

	ASSERT(length < sizeof(m_packet.raw_packet.data));
	memcpy(m_packet.raw_packet.data, buffer, length);

	this->updateChecksum();

	m_packet.raw_packet.end_word[0] = 0x0D;
	m_packet.raw_packet.end_word[1] = 0x0A;

}

uint16_t MTK::getDataLength(void) {
	return m_packet.data_count;
}

uint16_t MTK::getPacketLength(void) {
	return decode_uint16(m_packet.raw_packet.length);
}

/**
 *
 * @param buffer Dest buffer
 * @param max_size
 */
void MTK::toBuffer(uint8_t* buffer, uint16_t max_size) {

	memset(buffer, 0, max_size);

	ASSERT(this->getPacketLength() < max_size);

	memcpy(buffer, m_packet.raw_data, this->getDataLength()+6);

	memcpy(buffer+6+this->getDataLength(), m_packet.raw_data+MTK_MAX_PACKET_LENGTH-3, 3);

}

/**
 *
 * @return The packet command ID
 */
uint16_t MTK::getCommandId(void) {
	return decode_uint16(m_packet.raw_packet.command_id);
}

/**
 *
 * @return The computed checksum
 */
uint8_t MTK::calculateChecksum(void) {

	uint8_t ret = 0;

	ret ^= m_packet.raw_packet.length[0];
	ret ^= m_packet.raw_packet.length[1];

	ret ^= m_packet.raw_packet.command_id[0];
	ret ^= m_packet.raw_packet.command_id[1];

	for (uint16_t i = 0; i < this->getDataLength(); i++) {

		ret ^= m_packet.raw_packet.data[i];

	}

	return ret;
}

void MTK::updateChecksum(void) {

	m_packet.raw_packet.checksum = this->calculateChecksum();

	return;
}

/**
 *
 * @param c
 * @return
 */
uint8_t MTK::encode(char c) {

	static uint8_t last_rec = 0x00;

	if (!m_packet.byte_count &&
			c == 0x24 && last_rec == 0x04) {
		m_packet.raw_packet.preamble[0] = 0x04;
		m_packet.raw_packet.preamble[1] = 0x24;

		LOG_INFO("Packet started\r\n");

		m_packet.byte_count = 2;
	} else if (m_packet.byte_count) {

		//LOG_INFO("0x%02X ", c);

		if (m_packet.byte_count == 2) m_packet.raw_packet.length[0] = c;
		if (m_packet.byte_count == 3) {
			m_packet.raw_packet.length[1] = c;

			LOG_DEBUG("Size %X %X\r\n",
					m_packet.raw_packet.length[0], m_packet.raw_packet.length[1]);

			m_packet.data_count = decode_uint16(m_packet.raw_packet.length) - 9;
		}

		if (m_packet.byte_count == 4) m_packet.raw_packet.command_id[0] = c;
		if (m_packet.byte_count == 5) m_packet.raw_packet.command_id[1] = c;

		if (m_packet.byte_count >= MTK_DATA_START &&
				m_packet.byte_count < MTK_DATA_START + m_packet.data_count) {
			m_packet.raw_packet.data[m_packet.byte_count-MTK_DATA_START] = c;

		}

		if (m_packet.byte_count == MTK_DATA_START + m_packet.data_count) {
			m_packet.raw_packet.checksum = c;

			LOG_DEBUG("Checksum: 0x%02X\r\n", m_packet.raw_packet.checksum);

			m_packet.data_count = decode_uint16(m_packet.raw_packet.length) - 9;
		}

		if (m_packet.byte_count++ >= MTK_DATA_START + m_packet.data_count + 1) {
			if (c == 0x0A && last_rec == 0x0D) {
				m_packet.raw_packet.end_word[0] = 0x0D;
				m_packet.raw_packet.end_word[1] = 0x0A;

				m_packet.byte_count++;

				LOG_INFO("Command ID: 0x%04X\r\n", this->getCommandId());
				LOG_DEBUG("Checking %u bytes...\r\n", m_packet.data_count);

				if (m_packet.raw_packet.checksum == this->calculateChecksum()) {
					LOG_INFO("Packet finished: size=%u\r\n", m_packet.data_count);
					m_packet.is_valid = 1;
				} else {
					LOG_ERROR("Wrong checksum: 0x%X vs 0x%X\r\n",
							m_packet.raw_packet.checksum, this->calculateChecksum());
					this->clear();
				}
			}
		}
	}

	last_rec = c;

	if (m_packet.byte_count > 0xFF) {
		LOG_ERROR("Wrong byte count\r\n");
		this->clear();
		return 0;
	}

	return m_packet.is_valid;
}

uint8_t MTK::isValid(void) {
	return m_packet.is_valid;
}
