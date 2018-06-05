/*
 * Boucle.h
 *
 *  Created on: 19 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_BOUCLECRS_H_
#define SOURCE_MODEL_BOUCLECRS_H_

#include "UData.h"
#include <model/BoucleInterface.h>


class BoucleCRS : virtual public BoucleInterface {
public:
	BoucleCRS();
	void init();

	bool isTime();
	void run();

	uint16_t m_dist_next_seg;

protected:

	UData<uint8_t> m_last_refresh;
};



#endif /* SOURCE_MODEL_BOUCLECRS_H_ */
