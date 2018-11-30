/*
 * ring_buffer.h
 *
 *  Created on: 6 juin 2018
 *      Author: Vincent
 */

#ifndef LIBRARIES_UTILS_RING_BUFFER_H_
#define LIBRARIES_UTILS_RING_BUFFER_H_

#include <stdint.h>
#include "nordic_common.h"


typedef struct {
	uint8_t *p_buffer;
	uint16_t txIndex; /* Index of the data to send out. */
	uint16_t rxIndex; /* Index of the memory to save new arrived data. */
} sRingBufferDescriptor;


#define RING_BUFFER_DEF(_rb_name, _queue_size) \
		static const uint32_t CONCAT_2(_rb_name, _size) = _queue_size;                    \
		static uint8_t CONCAT_2(_rb_name, _cb[_queue_size]);                              \
		static volatile sRingBufferDescriptor _rb_name =                                  \
		{                                                                                 \
			.p_buffer                   = CONCAT_2(_rb_name, _cb),                        \
			.txIndex                    = 0,                                              \
			.rxIndex                    = 0                                               \
		}


#define RING_BUFF_GET_ELEM(_rb_name) \
		(_rb_name.p_buffer[_rb_name.txIndex])

#define RING_BUFF_GET_P_ELEM(_rb_name) \
		(&_rb_name.p_buffer[_rb_name.txIndex])

#define RING_BUFFER_ADD_ATOMIC(_rb_name, _elem) \
		{                                                                             \
			_rb_name.p_buffer[_rb_name.rxIndex] = _elem;                              \
			_rb_name.rxIndex++;														  \
			_rb_name.rxIndex %= _rb_name##_size;                                      \
		}

#define RING_BUFFER_ADD(_rb_name, _p_elem, _size) \
		{                                                                             \
			memcpy(&_rb_name.p_buffer[_rb_name.rxIndex], _p_elem, _size);             \
			_rb_name.rxIndex++;														  \
			_rb_name.rxIndex %= _rb_name##_size;                                      \
		}

#define RING_BUFF_IS_NOT_FULL(_rb_name) \
		(((_rb_name.rxIndex + 1) % _rb_name##_size) != _rb_name.txIndex)

#define RING_BUFF_IS_NOT_EMPTY(_rb_name) \
		(((_rb_name.txIndex + 0) % _rb_name##_size) != _rb_name.rxIndex)

#define RING_BUFF_EMPTY(_rb_name) \
		{                                       \
			_rb_name.txIndex=0;					\
			_rb_name.rxIndex=0;                 \
		}

#define RING_BUFFER_POP(_rb_name) \
		if (RING_BUFF_IS_NOT_EMPTY(_rb_name)) {                                       \
			_rb_name.txIndex++;														  \
			_rb_name.txIndex %= _rb_name##_size;                                        \
		}


#endif /* LIBRARIES_UTILS_RING_BUFFER_H_ */
