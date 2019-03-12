/*
 * bsc.h
 *
 *  Created on: 12 mrt. 2019
 *      Author: v.golle
 */

#ifndef RF_BSC_H_
#define RF_BSC_H_


#ifdef ANT_STACK_SUPPORT_REQD
#include "nrf_sdh_ant.h"
#endif

/////////////  FUNCTIONS

#ifdef ANT_STACK_SUPPORT_REQD

#ifdef __cplusplus
extern "C" {
#endif


void ant_evt_bsc (ant_evt_t * p_ant_evt);

void bsc_profile_setup(void);

void bsc_profile_start(void);


#ifdef __cplusplus
}
#endif

#endif


#endif /* RF_BSC_H_ */
