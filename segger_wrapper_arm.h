/*
 * segger_wrapper.h
 *
 *  Created on: 5 oct. 2017
 *      Author: Vincent
 */

#ifndef LIBRARIES_UTILS_SEGGER_WRAPPER_ARM_H_
#define LIBRARIES_UTILS_SEGGER_WRAPPER_ARM_H_

#include "nrf_log.h"
#include <stddef.h>

#ifndef EMPTY_MACRO
#define EMPTY_MACRO                    do {} while (0)
#endif

/////////    PARAMETERS

#ifndef USE_VCOM_LOGS
#define USE_VCOM_LOGS     0
#endif

#ifndef USE_SVIEW
#define USE_SVIEW         0
#endif


#ifndef NRF_LOG_BACKEND_RTT_ENABLED
#define NRF_LOG_BACKEND_RTT_ENABLED 1
#endif

#if USE_SVIEW
#include "SEGGER_SYSVIEW.h"
#endif

/////////    DEFINES

#define RTT_LOG_CHANNEL   0


#if defined( USB_ENABLED ) && USE_VCOM_LOGS
#include "usb_cdc.h"

#define USB_PRINTF(...)                  usb_printf(__VA_ARGS__)
#define USB_PRINTC( X )                  usb_print(X)

#else // defined( USB_ENABLED ) && USE_VCOM_LOGS

#define USB_PRINTF(...)                  EMPTY_MACRO
#define USB_PRINTC( X )                  EMPTY_MACRO

#endif

#if USE_SVIEW

#define SVIEW_INIT(...)                      segger_init()
#define LOG_WARNING_SVIEW(...)               SEGGER_SYSVIEW_WarnfHost(__VA_ARGS__)
#define LOG_ERROR_SVIEW(...)                 SEGGER_SYSVIEW_PrintfHost(__VA_ARGS__)

#else // USE_SVIEW

#define SVIEW_INIT(...)                      EMPTY_MACRO
#define LOG_WARNING_SVIEW(...)               EMPTY_MACRO
#define LOG_ERROR_SVIEW(...)               EMPTY_MACRO

#endif

#if NRF_LOG_ENABLED

#include "SEGGER_RTT.h"
#define LOG_INFO(...)                  SEGGER_RTT_printf(RTT_LOG_CHANNEL, __VA_ARGS__);SEGGER_RTT_PutChar(RTT_LOG_CHANNEL, '\r');SEGGER_RTT_PutChar(RTT_LOG_CHANNEL, '\n')
#define LOG_RAW_INFO(X)                EMPTY_MACRO
#define LOG_WARNING(...)               SEGGER_RTT_printf(RTT_LOG_CHANNEL, __VA_ARGS__);SEGGER_RTT_PutChar(RTT_LOG_CHANNEL, '\r');SEGGER_RTT_PutChar(RTT_LOG_CHANNEL, '\n')
#define LOG_DEBUG(...)                 EMPTY_MACRO
#define LOG_ERROR(...)                 SEGGER_RTT_printf(RTT_LOG_CHANNEL, __VA_ARGS__);SEGGER_RTT_PutChar(RTT_LOG_CHANNEL, '\r');SEGGER_RTT_PutChar(RTT_LOG_CHANNEL, '\n')
#define LOG_GRAPH(...)                 EMPTY_MACRO
#define LOG_FLUSH(...)                 EMPTY_MACRO
#define LOG_SET_TERM(X)                EMPTY_MACRO

#else // NRF_LOG_ENABLED

#define LOG_INFO(...)                  USB_PRINTF(__VA_ARGS__)
#define LOG_RAW_INFO(X)                USB_PRINTC(X)
#define LOG_WARNING(...)               LOG_WARNING_SVIEW(__VA_ARGS__);USB_PRINTF(__VA_ARGS__)
#define LOG_DEBUG(...)                 EMPTY_MACRO
#define LOG_ERROR(...)                 LOG_ERROR_SVIEW(__VA_ARGS__);USB_PRINTF(__VA_ARGS__)
#define LOG_GRAPH(...)                 EMPTY_MACRO
#define LOG_FLUSH(...)                 EMPTY_MACRO
#define LOG_SET_TERM(X)                EMPTY_MACRO

#endif


#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "task_manager_wrapper.h"


#endif /* LIBRARIES_UTILS_SEGGER_WRAPPER_H_ */
