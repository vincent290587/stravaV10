/*
 * mk64f_parser.h
 *
 *  Created on: 6 déc. 2017
 *      Author: Vincent
 */

#ifndef MK64F_PARSER_H_
#define MK64F_PARSER_H_

#include <stdint.h>

typedef enum {
	eSpiRxPageInv  = 0xFF,
	eSpiRxPage0    = 0U,
	eSpiRxPage1    = 1U,
} eSpisRxPages;

////////////// RX SPIS

typedef struct {
	uint8_t event_type;
	uint8_t on_time;
	uint8_t rgb[3];
} sNeopixelOrders;

typedef struct {
	uint8_t  led;
	uint8_t  av_ent;
	uint8_t  av_dec;
} sGlassesOrders;

typedef struct {
	uint8_t  soc;
	uint16_t mv;
} sBatteryOrders;

typedef enum {
	eFecControlNone,
	eFecControlTargetPower,
	eFecControlSlope,
} eFecControlType;

typedef struct {
	uint16_t target_power_w;
} sControlTargetPower;

typedef struct {
	float slope_ppc;
	float rolling_resistance;
} sControlSlope;

typedef union {
	sControlTargetPower power_control;
	sControlSlope       slope_control;
} uControlPages;

typedef struct {
	eFecControlType type;
	uControlPages   data;
} sFecControl;

typedef struct {
	uint8_t  state;
	uint8_t  freq;
} sBacklightOrders;

typedef struct {
	uint8_t  state;
} sPowerOrders;

////////////// TX SPIS

typedef enum {
	eMk64fInterrupt       = 1U,
	eMk64fLeftButton      = 2U,
	eMk64fCentralButton   = 3U,
	eMk64fRightButton     = 4U,
} eMk64fLineToggle;

typedef struct {
	uint8_t bpm;
	uint16_t rr;
} sHrmInfo;

typedef struct {
	uint32_t cadence;
	uint32_t speed;
} sBscInfo;

typedef struct {
	int32_t lat;
	int32_t lon;
	int32_t ele;
	int16_t speed;
	uint32_t secj;
	uint32_t date;
} sLnsInfo;

typedef struct {
	uint16_t power;
	uint16_t speed;
	uint16_t el_time;
} sFecInfo;

typedef struct {
	uint8_t  flags;
	sHrmInfo hrm_info;
	sBscInfo bsc_info;
	sLnsInfo lns_info;
	sFecInfo fec_info;
} sSpisTxInfoPage0;

////////////// NRF52 RX PAGES

typedef struct {
	sBatteryOrders  batt_info;
	sGlassesOrders  glasses_info;
	sNeopixelOrders neo_info;
	sFecControl     fec_info;
	sBacklightOrders back_info;
	sPowerOrders     power_info;
} sSpisRxInfoPage0;

typedef struct {
	uint8_t dummy;
} sSpisRxInfoPage1;

typedef union {
	sSpisRxInfoPage0 page0;
	sSpisRxInfoPage1 page1;
} sSpisRxPages;

typedef struct {
	eSpisRxPages page_id;
	sSpisRxPages pages;
} sSpisRxInfo;


#endif /* MK64F_PARSER_H_ */
