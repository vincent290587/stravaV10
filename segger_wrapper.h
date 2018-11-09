/*
 * segger_wrapper.h
 *
 *  Created on: 5 oct. 2017
 *      Author: Vincent
 */

#ifndef LIBRARIES_UTILS_SEGGER_WRAPPER_ARM_H_
#define LIBRARIES_UTILS_SEGGER_WRAPPER_ARM_H_

#define EMPTY_MACRO                    do {} while (0)

#define TASK_EVENT_1          (1 << 0)
#define TASK_EVENT_2          (1 << 1)
#define TASK_EVENT_3          (1 << 2)
#define TASK_EVENT_4          (1 << 3)
#define TASK_EVENT_5          (1 << 4)
#define TASK_EVENT_6          (1 << 5)
#define TASK_EVENT_7          (1 << 6)
#define TASK_EVENT_8          (1 << 7)

#define TASK_EVENT_LOCATION           TASK_EVENT_1
#define TASK_EVENT_FEC_INFO           TASK_EVENT_2
#define TASK_EVENT_FEC_POWER          TASK_EVENT_3
#define TASK_EVENT_BOUCLE_RELEASE     (TASK_EVENT_LOCATION | TASK_EVENT_FEC_INFO | TASK_EVENT_FEC_POWER)

#define TASK_EVENT_PERIPH_TRIGGER     TASK_EVENT_3

#define TASK_EVENT_LS027_TRIGGER      TASK_EVENT_4
#define TASK_EVENT_LS027_WAIT_SPI     TASK_EVENT_5


#ifdef TDD
#include "segger_wrapper_tdd.h"
#else
#include "segger_wrapper_arm.h"
#endif

#endif /* LIBRARIES_UTILS_SEGGER_WRAPPER_H_ */
