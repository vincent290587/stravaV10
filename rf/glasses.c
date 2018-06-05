#include <stdint.h>
#include <string.h>
#include "glasses.h"


// glasses profile
ant_glasses_profile_t       m_ant_glasses;

uint8_t m_glasses_payload[8];

/**
 *
 * @param orders
 */
void set_glasses_buffer (sGlassesOrders* orders) {

	memset(m_glasses_payload, 0, 8);

	m_glasses_payload[0] = orders->led;
	m_glasses_payload[1] = orders->av_ent;
	m_glasses_payload[2] = orders->av_dec;

}

