/*
 * Boucle.h
 *
 *  Created on: 19 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_BOUCLEINTERFACE_H_
#define SOURCE_MODEL_BOUCLEINTERFACE_H_

#include <stdint.h>
#include <stdbool.h>


class BoucleInterface {
public:
	BoucleInterface() {
		m_needs_init = true;
	}

	virtual void init_internal()=0;
	virtual void run_internal()=0;
	virtual void invalidate_internal(void)=0;

	void invalidate(void) {
		this->invalidate_internal();
		m_needs_init = true;
	}

	void run(void) {
		if (m_needs_init) {
			m_needs_init = false;
			this->init_internal();
		}
		this->run_internal();
	}

	bool needsInit() const {
		return m_needs_init;
	}

private:
	bool m_needs_init;

};


#endif /* SOURCE_MODEL_BOUCLEINTERFACE_H_ */
