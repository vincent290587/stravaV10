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
	eBoucleGlobalModesMSC,
} eBoucleGlobalModes;


extern BoucleCRS     boucle_crs;

extern BoucleFEC     boucle_fec;


void boucle__init(void);

void boucle__uninit(void);

void boucle__run(void);

void boucle__change_mode(eBoucleGlobalModes new_mode);

eBoucleGlobalModes boucle__get_mode(void);


#endif /* SOURCE_MODEL_BOUCLE_H_ */
