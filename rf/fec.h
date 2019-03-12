/*
 * fec.h
 *
 *  Created on: 9 déc. 2017
 *      Author: Vincent
 */

#ifndef FEC_H_
#define FEC_H_

#ifdef ANT_STACK_SUPPORT_REQD
#include "ant_fec.h"
#include "nrf_sdh_ant.h"
#endif
#include "mk64f_parser.h"
#include "g_structs.h"

#define FEC_CHANNEL_NUMBER              0x03
#define TACX_DEVICE_NUMBER              2846U

/////////////  STRUCTS

extern sFecControl          fec_control;

extern sFecInfo             fec_info;

#ifdef ANT_STACK_SUPPORT_REQD

extern ant_fec_profile_t        m_ant_fec;

extern ant_fec_message_layout_t m_fec_message_payload;

#endif

/////////////  FUNCTIONS

#ifdef ANT_STACK_SUPPORT_REQD

#ifdef __cplusplus
extern "C" {
#endif

void fec_init(void);

void fec_set_control(sFecControl* tbc);

void roller_manager_tasks(void);

void ant_evt_fec (ant_evt_t * p_ant_evt);

void fec_profile_setup(void);

void fec_profile_start(void);

#ifdef __cplusplus
}
#endif

#endif

#endif /* FEC_H_ */
