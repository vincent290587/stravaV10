/*
 * RingBuffer.h
 *
 *  Created on: 27 nov. 2017
 *      Author: Vincent
 */

#ifndef HELPER_RINGBUFFER_H_
#define HELPER_RINGBUFFER_H_

#include <stdint.h>
#include <string.h>
#include <segger_wrapper.h>
#include "assert_wrapper.h"

typedef struct {
	uint16_t size;
	uint16_t cur_index;
	uint16_t tx_index;
} sRingBufferQueue;


template <typename T>
class RingBuffer {
public:
	RingBuffer(uint16_t size, T* buffer_);
	virtual ~RingBuffer();

	void add(T* elem);
	void empty(void);

	bool isFull(void);
	bool isEmpty(void);

	void popLast();

	uint16_t size(void);

	T* get(uint16_t i=0);

protected:
	T* m_objects;

	sRingBufferQueue m_queue;
};

template<typename T>
inline RingBuffer<T>::RingBuffer(uint16_t size_, T* buffer_) {

	memset(&m_queue, 0, sizeof(m_queue));

	m_queue.size    = size_;
	m_objects       = buffer_;

	ASSERT(m_queue.size);
}

template<typename T>
inline RingBuffer<T>::~RingBuffer() {
}


template<typename T>
inline void RingBuffer<T>::empty(void) {

	m_queue.cur_index = 0;
	m_queue.tx_index = 0;

}

template<typename T>
inline bool RingBuffer<T>::isFull() {
	if (((m_queue.cur_index + 1) % m_queue.size) != m_queue.tx_index) {
		return false;
	}

	return true;
}

template<typename T>
inline bool RingBuffer<T>::isEmpty() {
	if (((m_queue.tx_index + 0) % m_queue.size) != m_queue.cur_index) {
		return false;
	}

	return true;
}

template<typename T>
inline uint16_t RingBuffer<T>::size(void) {

	uint16_t nb_elem;

	ASSERT(m_queue.size);

	nb_elem = m_queue.cur_index + m_queue.size;
	nb_elem -= m_queue.tx_index;
	nb_elem %= m_queue.size;

	return nb_elem;
}

template<typename T>
inline void RingBuffer<T>::add(T* elem) {

	if (this->isFull()) {
		LOG_ERROR("RingBuffer full");
		return;
	}

	uint16_t index = m_queue.cur_index;

	ASSERT(m_queue.size);

	// queue
	memcpy(&m_objects[index], elem, sizeof(T));

	m_queue.cur_index++;
	m_queue.cur_index %= m_queue.size;

}

template<typename T>
inline void RingBuffer<T>::popLast(void) {

	if (this->isEmpty()) {
		LOG_ERROR("RingBuffer empty");
		return;
	}

	ASSERT(m_queue.size);

	// dequeue
	m_queue.tx_index++;
	m_queue.tx_index %= m_queue.size;

}

template<typename T>
inline T* RingBuffer<T>::get(uint16_t i) {
	uint16_t index = m_queue.tx_index + i;
	index %= m_queue.size;

	return m_objects + index;
}

#endif /* HELPER_RINGBUFFER_H_ */
