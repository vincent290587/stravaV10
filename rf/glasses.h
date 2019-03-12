
#ifndef GLASSES_H_
#define GLASSES_H_

#include "mk64f_parser.h"
#ifdef ANT_STACK_SUPPORT_REQD
#include "ant_glasses.h"
#include "nrf_sdh_ant.h"

#define GLASSES_CHANNEL_NUMBER          0x02

extern ant_glasses_profile_t       m_ant_glasses;

extern uint8_t m_glasses_payload[8];

#ifdef __cplusplus
extern "C" {
#endif

void glasses_init(void);

void set_glasses_buffer (sGlassesOrders* orders);

void ant_evt_glasses (ant_evt_t * p_ant_evt);

void glasses_profile_setup(void);

void glasses_profile_start(void);

#ifdef __cplusplus
}
#endif

#endif

#endif /* GLASSES_H_ */
