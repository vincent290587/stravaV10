
#ifndef NOTIFICATIONS_H_
#define NOTIFICATIONS_H_

#include <stdint.h>
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


//////////////////////////     MACROS


#define SET_NEO_EVENT_RED(X, Y, TIME) \
	X.event_type = Y; \
	X.on_time = 5; \
	X.rgb[0] = 0xFF; \
	X.rgb[1] = 0x00; \
	X.rgb[2] = 0x00

#define SET_NEO_EVENT_BLUE(X, Y, TIME) \
	X.event_type = Y; \
	X.on_time = 5; \
	X.rgb[0] = 0x00; \
	X.rgb[1] = 0xFF; \
	X.rgb[2] = 0x00

#define SET_NEO_EVENT_GREEN(X, Y, TIME) \
	X.event_type = Y; \
	X.on_time = 5; \
	X.rgb[0] = 0x00; \
	X.rgb[1] = 0x00; \
	X.rgb[2] = 0xFF

//////////////////////////     FUNCTIONS

#ifdef __cplusplus
extern "C" {
#endif

void notifications_init(uint8_t pin_num);

void notifications_setNotify(sNeopixelOrders* orders);

uint8_t notifications_tasks(void);

#ifdef __cplusplus
}
#endif

#endif /* NOTIFICATIONS_H_ */
