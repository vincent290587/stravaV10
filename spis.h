/*
 * spis.h
 *
 *  Created on: 6 déc. 2017
 *      Author: Vincent
 */

#ifndef SPIS_H_
#define SPIS_H_

#define SPIS_BUFFER_SIZE     64

extern uint8_t* m_tx_buf;

#ifdef __cplusplus
extern "C" {
#endif

void spis_init(void);

void spis_tasks(void);

#ifdef __cplusplus
}
#endif

#endif /* SPIS_H_ */
