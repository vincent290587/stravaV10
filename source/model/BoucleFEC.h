/*
 * BoucleFEC.h
 *
 *  Created on: 13 déc. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_BOUCLEFEC_H_
#define SOURCE_MODEL_BOUCLEFEC_H_

#include <model/BoucleInterface.h>
#include "VueCommon.h"
#include "RingBuffer.h"

class BoucleFEC: virtual public BoucleInterface {
public:
	BoucleFEC();

	virtual void init_internal(void);
	virtual void run_internal(void);
	virtual void invalidate_internal(void);

	RingBuffer<tHistoValue> m_pw_buffer;

};

#endif /* SOURCE_MODEL_BOUCLEFEC_H_ */
