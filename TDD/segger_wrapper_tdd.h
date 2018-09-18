/*
 * segger_wrapper_tdd.h
 *
 *  Created on: 17 sept. 2018
 *      Author: Vincent
 */

#ifndef SEGGER_WRAPPER_TDD_H_
#define SEGGER_WRAPPER_TDD_H_

#include <stdio.h>

#define LOG_INFO(...)                  printf(__VA_ARGS__);printf("\n")
#define LOG_RAW_INFO(X)                EMPTY_MACRO
#define LOG_WARNING(...)               printf(__VA_ARGS__);printf("\n")
#define LOG_DEBUG(...)                 EMPTY_MACRO
#define LOG_ERROR(...)                 printf("\n");printf(__VA_ARGS__);printf("\n");printf("\n")
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


#define W_SYSVIEW_RecordEnterISR(...)  EMPTY_MACRO
#define W_SYSVIEW_RecordExitISR(...)   EMPTY_MACRO
#define W_SYSVIEW_OnIdle(...)          EMPTY_MACRO
#define W_SYSVIEW_OnTaskStartExec(X)   EMPTY_MACRO
#define W_SYSVIEW_OnTaskStopExec(X)    EMPTY_MACRO
#define W_SYSVIEW_OnTaskCreate(X)      EMPTY_MACRO

#endif /* SEGGER_WRAPPER_TDD_H_ */
