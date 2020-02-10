/*
 * Boucle.h
 *
 *  Created on: 19 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_BOUCLECRS_H_
#define SOURCE_MODEL_BOUCLECRS_H_

#include "UData.h"
#include "Parcours.h"
#include <model/BoucleInterface.h>


class BoucleCRS : virtual public BoucleInterface {
public:
	BoucleCRS();

	void init_internal(void);

	void run_internal(void);

	void invalidate_internal(void);

	void loadPRC();

	void parcoursSelect(int prc_ind);

	uint16_t m_dist_next_seg;

	Parcours *m_s_parcours;
};



#endif /* SOURCE_MODEL_BOUCLECRS_H_ */
