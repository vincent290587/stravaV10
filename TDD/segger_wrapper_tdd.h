/*
 * segger_wrapper_tdd.h
 *
 *  Created on: 17 sept. 2018
 *      Author: Vincent
 */

#ifndef SEGGER_WRAPPER_TDD_H_
#define SEGGER_WRAPPER_TDD_H_


#ifdef	__cplusplus
#include <cstdio>
#define WPRINTF            std::printf
#else
#include <stdio.h>
#define WPRINTF            printf
#endif




#define LOG_INFO(...)                  WPRINTF(__VA_ARGS__);WPRINTF("\n")
#define LOG_RAW_INFO(X)                EMPTY_MACRO
#define LOG_WARNING(...)               WPRINTF(__VA_ARGS__);WPRINTF("\n")
#define LOG_DEBUG(...)                 EMPTY_MACRO
#define LOG_ERROR(...)                 WPRINTF("\n");WPRINTF(__VA_ARGS__);WPRINTF("\n");WPRINTF("\n")
#define LOG_GRAPH(...)                 EMPTY_MACRO
#define LOG_FLUSH(...)                 EMPTY_MACRO
#define LOG_SET_TERM(X)                EMPTY_MACRO
#define SVIEW_INIT(...)                EMPTY_MACRO
#define USB_PRINTF(...)                EMPTY_MACRO
#define USB_PRINT(...)                 EMPTY_MACRO
#define USB_PRINTF(...)                EMPTY_MACRO
#define USB_PRINT(...)                 EMPTY_MACRO
#define NRF_LOG_DEBUG(...)             EMPTY_MACRO
#define NRF_LOG_ERROR(...)             LOG_ERROR(__VA_ARGS__)
#define NRF_LOG_FLUSH(...)             EMPTY_MACRO
#define NRF_LOG_WARNING(...)           LOG_WARNING(__VA_ARGS__)
#define NRF_LOG_PROCESS(...)           false

#define W_SYSVIEW_RecordEnterISR(...)  EMPTY_MACRO
#define W_SYSVIEW_RecordExitISR(...)   EMPTY_MACRO
#define W_SYSVIEW_OnIdle(...)          EMPTY_MACRO
#define W_SYSVIEW_OnTaskStartExec(X)   EMPTY_MACRO
#define W_SYSVIEW_OnTaskStopExec(X)    EMPTY_MACRO
#define W_SYSVIEW_OnTaskCreate(X)      EMPTY_MACRO
#define W_SYSVIEW_OnTaskStartReady(X)    EMPTY_MACRO
#define W_SYSVIEW_OnTaskStopReady(X, M)  EMPTY_MACRO

#include "task_manager_wrapper_tdd.h"

#ifdef	__cplusplus
extern "C" {
#endif

void millis_increase_time(int dt);

#ifdef	__cplusplus
}
#endif

#endif /* SEGGER_WRAPPER_TDD_H_ */
