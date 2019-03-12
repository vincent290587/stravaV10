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

#define HRM_CHANNEL_NUMBER              0x00
#define HRM_DEVICE_NUMBER               0x0D22    /**< Device Number. */

/////////////  FUNCTIONS

#ifdef ANT_STACK_SUPPORT_REQD

#ifdef __cplusplus
extern "C" {
#endif

void hrm_init(void);

void ant_evt_hrm (ant_evt_t * p_ant_evt);

void hrm_profile_setup(void);

void hrm_profile_start(void);


#ifdef __cplusplus
}
#endif

#endif


#endif /* RF_HRM_H_ */
