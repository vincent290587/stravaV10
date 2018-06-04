/*
 * WS2812B.h
 *
 * Wrapper for the neopixel LED
 *
 *  Created on: 28 févr. 2017
 *      Author: Vincent
 */

#ifndef LIBRARIES_WS2812B_H_
#define LIBRARIES_WS2812B_H_

#include <stdint.h>
#include "neopixel.h"
#include "mk64f_parser.h"

typedef enum {
	eNeoEventEmpty       = 0U,
	eNeoEventWeakNotify  = 1U,
	eNeoEventNotify      = 2U,
} eNeoEventType;

typedef struct
{
    uint8_t         max;  /**< Maximum */
    uint8_t         min;  /**< Minimum */
    uint8_t         step; /**< step. */
    uint8_t         rgb[3];
    uint32_t        on_time_ticks;   /**< Ticks to stay in high impulse state. */
} neo_sb_init_params_t;


#ifdef __cplusplus
extern "C" {
#endif

void notifications_init(uint8_t pin_num);

void notifications_setNotify(sNeopixelOrders* orders);

uint8_t notifications_tasks(void);

#ifdef __cplusplus
}
#endif

#endif /* LIBRARIES_WS2812B_H_ */
