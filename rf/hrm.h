/*
 * hrm.h
 *
 *  Created on: 12 mrt. 2019
 *      Author: v.golle
 */

#ifndef RF_HRM_H_
#define RF_HRM_H_

#ifdef ANT_STACK_SUPPORT_REQD
#include "nrf_sdh_ant.h"
#endif

/////////////  FUNCTIONS

#ifdef ANT_STACK_SUPPORT_REQD

#ifdef __cplusplus
extern "C" {
#endif


void ant_evt_hrm (ant_evt_t * p_ant_evt);

void hrm_profile_setup(void);

void hrm_profile_start(void);


#ifdef __cplusplus
}
#endif

#endif


#endif /* RF_HRM_H_ */
