/*
 * JScope.h
 *
 *  Created on: 20 sept. 2017
 *      Author: Vincent
 */

#ifndef HELPER_JSCOPE_H_
#define HELPER_JSCOPE_H_


#include "helper.h"

#define JSCOPE_INT_BUFFER   32

#ifdef __cplusplus

/**
 * For this class to work, you need to define the header correctly.
 *
 * It must start with "JScope_" and then indicate the type of data you want to transmit.
 * For exemple i2u1u4 would be a int16_t, followed by a uint8_t followed be a uint32_t.
 * You can use the letters i & u, followed by an integer in {1,2,4}.
 *
 * To activate the functionality you have to add "override CFLAGS += -DJSCOPE -DJSCOPE_XXX -DNRF_LOG_ENABLED=0"
 * to the makefile, with "JSCOPE_XXX" being the JSCOPE of the target module
 */
class JScope {
public:
	JScope();
	void init();
	void inputData(uint8_t data, uint8_t pos);
	void inputData(int16_t data, uint8_t pos);
	void inputData(int32_t data, uint8_t pos);
	void inputData(float data, uint8_t pos);
	void inputData(uint8_t *data, uint8_t pos, uint8_t length);
	void flush();

private:
	uint8_t m_buffer[JSCOPE_INT_BUFFER];
	bool m_is_init = false;
};
#endif

#endif /* HELPER_JSCOPE_H_ */
