/*
 * task_manager_wrapper.h
 *
 *  Created on: 30 oct. 2018
 *      Author: Vincent
 */

#ifndef TASK_MANAGER_WRAPPER_H_
#define TASK_MANAGER_WRAPPER_H_


#if defined(__cplusplus)
extern "C" {
#endif /* _cplusplus */


void segger_init(void);

void sysview_task_block(uint32_t);
void sysview_task_transfer(uint32_t);
void sysview_task_event(uint32_t, uint32_t);
void sysview_task_void_enter(uint32_t);
void sysview_task_void_exit(void);
void sysview_task_idle(void);


#if defined(__cplusplus)
}
#endif /* _cplusplus */


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
#define TASK_EVENT_PERIPH_TWI_WAIT    TASK_EVENT_6
#define TASK_EVENT_PERIPH_MS_WAIT     TASK_EVENT_7

#define TASK_EVENT_LS027_TRIGGER      TASK_EVENT_4
#define TASK_EVENT_LS027_WAIT_SPI     TASK_EVENT_5



#define SYSVIEW_MAX_NOF_TASKS          30

#define TASK_BASE_NRF                  (36u)

#define BOUCLE_TASK                    (TASK_BASE_NRF + 0u)
#define SYSTEM_TASK                    (TASK_BASE_NRF + 1u)
#define PERIPH_TASK                    (TASK_BASE_NRF + 2u)
#define LCD_TASK                       (TASK_BASE_NRF + 3u)

#define TASK_RECV_EVENT                499u

#define TASK_BASE                      (500u)

#define I2C_TASK                       (TASK_BASE + 0u)
#define SPI_TASK                       (TASK_BASE + 1u)
#define UART_TASK                      (TASK_BASE + 2u)

//#define ANT_TASK                       (TASK_BASE + 6u)
#define BLE_TASK                       (TASK_BASE + 7u)
#define SD_ACCESS_TASK                 (TASK_BASE + 8u)
#define SEG_PERF_TASK                  (TASK_BASE + 9u)
#define NRF52_TASK                     (TASK_BASE + 10u)
#define DISPLAY_TASK3                  (TASK_BASE + 12u)
#define DISPLAY_TASK4                  (TASK_BASE + 13u)
#define USB_VCOM_TASK                  (TASK_BASE + 14u)
#define SST_TASK                       (TASK_BASE + 15u)
#define EMPTY1                         (TASK_BASE + 16u)
#define EMPTY2                         (TASK_BASE + 17u)


#ifdef TDD
#include "task_manager_wrapper_tdd.h"
#else
#include "task_manager_wrapper_arm.h"
#endif


#endif /* TASK_MANAGER_WRAPPER_H_ */
