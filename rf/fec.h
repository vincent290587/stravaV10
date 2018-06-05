/*
 * fec.h
 *
 *  Created on: 9 déc. 2017
 *      Author: Vincent
 */

#ifndef FEC_H_
#define FEC_H_

#include "ant_fec.h"
#include "nrf_sdh_ant.h"
#include "mk64f_parser.h"

/////////////  STRUCTS


extern sFecControl              m_fec_control;

extern ant_fec_profile_t        m_ant_fec;

extern ant_fec_message_layout_t m_fec_message_payload;

/////////////  FUNCTIONS

#ifdef __cplusplus
extern "C" {
#endif

void fec_init(void);

void fec_set_control(sFecControl* tbc);

void roller_manager_tasks(void);

void ant_evt_fec (ant_evt_t * p_ant_evt);

void ant_fec_evt_handler(ant_fec_profile_t * p_profile, ant_fec_evt_t event);

#ifdef __cplusplus
}
#endif

#endif /* FEC_H_ */
