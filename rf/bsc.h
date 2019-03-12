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

#define BSC_CHANNEL_NUMBER              0x01
#define BSC_DEVICE_NUMBER               0xB02B    /**< Device Number. */
#define BSC_DEVICE_TYPE                 0x79

/////////////  FUNCTIONS

#ifdef ANT_STACK_SUPPORT_REQD

#ifdef __cplusplus
extern "C" {
#endif

void bsc_init(void);

void ant_evt_bsc (ant_evt_t * p_ant_evt);

void bsc_profile_setup(void);

void bsc_profile_start(void);


#ifdef __cplusplus
}
#endif

#endif


#endif /* RF_BSC_H_ */
