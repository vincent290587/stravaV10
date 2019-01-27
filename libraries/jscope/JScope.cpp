/*
 * JScope.cpp
 *
 *  Created on: 20 sept. 2017
 *      Author: Vincent
 */

#include "JScope.h"
#include "SEGGER_RTT.h"

#define JSCOPE_LEN          16
#define JSCOPE_HEADER       "JScope_f4f4f4f4"

/**
 * Constructor
 */
JScope::JScope() {

	memset(m_buffer, 0, JSCOPE_INT_BUFFER);
}

/** @brief Writes data to the internal buffer
 *
 * @param data An array of uint8_t elements
 * @param pos The position to write to the internal buffer
 * @param length The length of the array
 */
void JScope::inputData(uint8_t *data, uint8_t pos, uint8_t length) {

	if (pos + length <= JSCOPE_LEN) {
		memcpy(m_buffer + pos, data, length);
	}

}

/** @brief Writes data to the internal buffer
 *
 * @param data A single int16_t element
 * @param pos The position to write to the internal buffer
 */
void JScope::inputData(int16_t data, uint8_t pos) {

	int16_t _buf[2];

	_buf[0] = data;

	this->inputData((uint8_t*)_buf, pos, 2);
}

/** @brief Writes data to the internal buffer
 *
 * @param data A single int32_t element
 * @param pos The position to write to the internal buffer
 */
void JScope::inputData(int32_t data, uint8_t pos) {

	int32_t _buf[2];

	_buf[0] = data;

	this->inputData((uint8_t*)_buf, pos, 4);
}

/** @brief Writes data to the internal buffer
 *
 * @param data A single uint8_t element
 * @param pos The position to write to the internal buffer
 */
void JScope::inputData(uint8_t data, uint8_t pos) {

	if (pos < JSCOPE_LEN) {
		m_buffer[pos] = data;
	}

}

/** @brief Writes data to the internal buffer
 *
 * @param data A single float element
 * @param pos The position to write to the internal buffer
 */
void JScope::inputData(float data, uint8_t pos) {

	float _buf[2];

	_buf[0] = data;

	this->inputData((uint8_t*)_buf, pos, 4);
}

void JScope::init() {

	SEGGER_RTT_Init();
	SEGGER_RTT_SetNameUpBuffer(0, JSCOPE_HEADER);
	m_is_init = true;
}

/**
 * Flushes the buffers and sends to the computer for display
 */
void JScope::flush() {

	if (!m_is_init) {
		this->init();
	}

	SEGGER_RTT_Write(0, (const void*)m_buffer, JSCOPE_LEN);

	memset(m_buffer, 0, JSCOPE_INT_BUFFER);
}
