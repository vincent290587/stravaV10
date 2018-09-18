/*
 * serial.h
 *
 *  Created on: 7 déc. 2017
 *      Author: Vincent
 */

#ifndef SERIAL_TDD_H_
#define SERIAL_TDD_H_

#include "stddef.h"
#include "stdint.h"

/* Bits 31..0 : Baud rate */
#define UARTE_BAUDRATE_BAUDRATE_Pos (0UL) /*!< Position of BAUDRATE field. */
#define UARTE_BAUDRATE_BAUDRATE_Msk (0xFFFFFFFFUL << UARTE_BAUDRATE_BAUDRATE_Pos) /*!< Bit mask of BAUDRATE field. */
#define UARTE_BAUDRATE_BAUDRATE_Baud1200 (0x0004F000UL) /*!< 1200 baud (actual rate: 1205) */
#define UARTE_BAUDRATE_BAUDRATE_Baud2400 (0x0009D000UL) /*!< 2400 baud (actual rate: 2396) */
#define UARTE_BAUDRATE_BAUDRATE_Baud4800 (0x0013B000UL) /*!< 4800 baud (actual rate: 4808) */
#define UARTE_BAUDRATE_BAUDRATE_Baud9600 (0x00275000UL) /*!< 9600 baud (actual rate: 9598) */
#define UARTE_BAUDRATE_BAUDRATE_Baud14400 (0x003AF000UL) /*!< 14400 baud (actual rate: 14401) */
#define UARTE_BAUDRATE_BAUDRATE_Baud19200 (0x004EA000UL) /*!< 19200 baud (actual rate: 19208) */
#define UARTE_BAUDRATE_BAUDRATE_Baud28800 (0x0075C000UL) /*!< 28800 baud (actual rate: 28777) */
#define UARTE_BAUDRATE_BAUDRATE_Baud31250 (0x00800000UL) /*!< 31250 baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud38400 (0x009D0000UL) /*!< 38400 baud (actual rate: 38369) */
#define UARTE_BAUDRATE_BAUDRATE_Baud56000 (0x00E50000UL) /*!< 56000 baud (actual rate: 55944) */
#define UARTE_BAUDRATE_BAUDRATE_Baud57600 (0x00EB0000UL) /*!< 57600 baud (actual rate: 57554) */
#define UARTE_BAUDRATE_BAUDRATE_Baud76800 (0x013A9000UL) /*!< 76800 baud (actual rate: 76923) */
#define UARTE_BAUDRATE_BAUDRATE_Baud115200 (0x01D60000UL) /*!< 115200 baud (actual rate: 115108) */
#define UARTE_BAUDRATE_BAUDRATE_Baud230400 (0x03B00000UL) /*!< 230400 baud (actual rate: 231884) */
#define UARTE_BAUDRATE_BAUDRATE_Baud250000 (0x04000000UL) /*!< 250000 baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud460800 (0x07400000UL) /*!< 460800 baud (actual rate: 457143) */
#define UARTE_BAUDRATE_BAUDRATE_Baud921600 (0x0F000000UL) /*!< 921600 baud (actual rate: 941176) */
#define UARTE_BAUDRATE_BAUDRATE_Baud1M (0x10000000UL) /*!< 1Mega baud */


/**
 * @enum nrf_uarte_baudrate_t
 * @brief Baudrates supported by UARTE.
 */
typedef enum
{
    NRF_UARTE_BAUDRATE_1200    = UARTE_BAUDRATE_BAUDRATE_Baud1200,   ///< 1200 baud.
    NRF_UARTE_BAUDRATE_2400    = UARTE_BAUDRATE_BAUDRATE_Baud2400,   ///< 2400 baud.
    NRF_UARTE_BAUDRATE_4800    = UARTE_BAUDRATE_BAUDRATE_Baud4800,   ///< 4800 baud.
    NRF_UARTE_BAUDRATE_9600    = UARTE_BAUDRATE_BAUDRATE_Baud9600,   ///< 9600 baud.
    NRF_UARTE_BAUDRATE_14400   = UARTE_BAUDRATE_BAUDRATE_Baud14400,  ///< 14400 baud.
    NRF_UARTE_BAUDRATE_19200   = UARTE_BAUDRATE_BAUDRATE_Baud19200,  ///< 19200 baud.
    NRF_UARTE_BAUDRATE_28800   = UARTE_BAUDRATE_BAUDRATE_Baud28800,  ///< 28800 baud.
    NRF_UARTE_BAUDRATE_31250   = UARTE_BAUDRATE_BAUDRATE_Baud31250,  ///< 31250 baud.
    NRF_UARTE_BAUDRATE_38400   = UARTE_BAUDRATE_BAUDRATE_Baud38400,  ///< 38400 baud.
    NRF_UARTE_BAUDRATE_56000   = UARTE_BAUDRATE_BAUDRATE_Baud56000,  ///< 56000 baud.
    NRF_UARTE_BAUDRATE_57600   = UARTE_BAUDRATE_BAUDRATE_Baud57600,  ///< 57600 baud.
    NRF_UARTE_BAUDRATE_76800   = UARTE_BAUDRATE_BAUDRATE_Baud76800,  ///< 76800 baud.
    NRF_UARTE_BAUDRATE_115200  = UARTE_BAUDRATE_BAUDRATE_Baud115200, ///< 115200 baud.
    NRF_UARTE_BAUDRATE_230400  = UARTE_BAUDRATE_BAUDRATE_Baud230400, ///< 230400 baud.
    NRF_UARTE_BAUDRATE_250000  = UARTE_BAUDRATE_BAUDRATE_Baud250000, ///< 250000 baud.
    NRF_UARTE_BAUDRATE_460800  = UARTE_BAUDRATE_BAUDRATE_Baud460800, ///< 460800 baud.
    NRF_UARTE_BAUDRATE_921600  = UARTE_BAUDRATE_BAUDRATE_Baud921600, ///< 921600 baud.
    NRF_UARTE_BAUDRATE_1000000 = UARTE_BAUDRATE_BAUDRATE_Baud1M      ///< 1000000 baud.
} nrf_uarte_baudrate_t;

/**
 * @brief Types of UARTE driver events.
 */
typedef enum
{
    NRFX_UARTE_EVT_TX_DONE, ///< Requested TX transfer completed.
    NRFX_UARTE_EVT_RX_DONE, ///< Requested RX transfer completed.
    NRFX_UARTE_EVT_ERROR,   ///< Error reported by UART peripheral.
} nrfx_uarte_evt_type_t;

#ifdef __cplusplus
extern "C" {
#endif

void uart_timer_init(void);

void uart_init(nrf_uarte_baudrate_t baud);

void uart_init_tx_only(nrf_uarte_baudrate_t baud);

void uart_uninit(void);

void uart_tasks(void);

void uart_send(uint8_t * p_data, size_t length);

void uart_rx_handler(char c);

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_H_ */
