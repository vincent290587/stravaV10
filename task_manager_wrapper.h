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
void sysview_task_u32_enter(uint32_t, uint32_t);
void sysview_task_void_exit(uint32_t);
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
#define PERIPH_TASK                    (TASK_BASE_NRF + 1u)
#define LCD_TASK                       (TASK_BASE_NRF + 2u)
#define SYSTEM_TASK                    (TASK_BASE_NRF + 3u)

#define TASK_BASE                      (512u)

#define TASK_RECV_EVENT                (TASK_BASE + 0u)

#define I2cReadSensors                 (TASK_BASE + 1u)
#define SpiSendBuffer                  (TASK_BASE + 2u)
#define UART_TASK                      (TASK_BASE + 3u)

#define MainSegLoop                    (TASK_BASE + 4u)
#define SdAccess                       (TASK_BASE + 5u)
#define ComputeSegmentPerf             (TASK_BASE + 6u)
#define ComputeZoom                    (TASK_BASE + 7u)
#define DisplayPoints                  (TASK_BASE + 8u)
#define DisplayMyself                  (TASK_BASE + 9u)
#define SaveUserPosition               (TASK_BASE + 10u)
#define USB_VCOM_TASK                  (TASK_BASE + 11u)
#define SST_TASK                       (TASK_BASE + 12u)
#define I2cReadReg8                    (TASK_BASE + 13u)
#define I2cReadRegN                    (TASK_BASE + 14u)
#define I2cMgmtReadMs                  (TASK_BASE + 15u)
#define I2cMgmtRead1                  (TASK_BASE + 16u)
#define I2cMgmtRead2                  (TASK_BASE + 17u)


#ifdef TDD
#include "task_manager_wrapper_tdd.h"
#else
#include "task_manager_wrapper_arm.h"
#endif


#endif /* TASK_MANAGER_WRAPPER_H_ */
