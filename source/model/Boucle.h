/*
 * Boucle.h
 *
 *  Created on: 13 déc. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_BOUCLE_H_
#define SOURCE_MODEL_BOUCLE_H_

#include <model/BoucleInterface.h>
#include <model/BoucleCRS.h>
#include <model/BoucleFEC.h>
#include "parameters.h"

typedef enum {
	eBoucleGlobalModesInit,
	eBoucleGlobalModesCRS,
	eBoucleGlobalModesFEC,
	eBoucleGlobalModesPRC,
} eBoucleGlobalModes;


extern BoucleCRS     boucle_crs;

extern BoucleFEC     boucle_fec;


class Boucle : virtual public BoucleInterface {
public:
	Boucle();

	void init(void);

	bool isTime();

	void changeMode(eBoucleGlobalModes new_mode);
	void run(void);

protected:
	eBoucleGlobalModes m_global_mode;
};

#endif /* SOURCE_MODEL_BOUCLE_H_ */
