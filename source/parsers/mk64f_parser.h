/*
 * mk64f_parser.h
 *
 *  Created on: 6 dec. 2017
 *      Author: Vincent
 */

#ifndef MK64F_PARSER_H_
#define MK64F_PARSER_H_

#include <stdint.h>


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
	eFecControlTargetNone,
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


#endif /* MK64F_PARSER_H_ */
