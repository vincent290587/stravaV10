/*
 * spis.c
 *
 *  Created on: 6 déc. 2017
 *      Author: Vincent
 */

#include "sdk_config.h"
#include "nrf_drv_spis.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_pwr_mgmt.h"
#include <string.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"



uint8_t*             m_tx_buf;


static volatile bool spis_xfer_done; /**< Flag used to indicate that SPIS instance completed the transfer. */


/**
 * @brief SPIS user event handler.
 *
 * @param event
 */
static void spis_event_handler(nrf_drv_spis_event_t event)
{

}

/**
 *
 */
void spis_init(void) {

}

/**
 *
 */
void spis_tasks(void) {

}
