/**
 * Copyright (c) 2014 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */


#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "ble_nus.h"
#include "ble_dis.h"
#include "ble_bas.h"
#include "ble_lns.h"
#include "app_timer.h"
#include "app_util_platform.h"

#include "task_manager.h"

#include "ble_api_base.h"
#include "lezyne_protocol.h"
#include "app_packets_handler.h"

#include "peer_manager.h"
#include "peer_manager_handler.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define ADV_FOR_IPHONE     0

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define DEVICE_NAME                     "LE GPS 12"                                 /**< Name of device. Will be included in the advertising data. */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL                64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */

#define APP_ADV_DURATION                18000                                       /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */

#define LESC_DEBUG_MODE                     0                                       /**< Set to 1 to use LESC debug keys, allows you to use a sniffer to inspect traffic. */

#define SEC_PARAM_BOND                      1                                       /**< Perform bonding. */
#define SEC_PARAM_MITM                      0                                       /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                      1                                       /**< LE Secure Connections enabled. */
#define SEC_PARAM_KEYPRESS                  0                                       /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES           BLE_GAP_IO_CAPS_NONE                    /**< No I/O capabilities. */
#define SEC_PARAM_OOB                       0                                       /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE              7                                       /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE              16                                      /**< Maximum encryption key size. */

BLE_LNS_DEF(m_lns);                                         /**< Location and navigation service instance. */
BLE_BAS_DEF(m_bas);
BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);                                   /**< BLE NUS service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */
NRF_BLE_GQ_DEF(m_ble_gatt_queue,                            /**< BLE GATT queue instance. */
        NRF_SDH_BLE_PERIPHERAL_LINK_COUNT,
        NRF_BLE_GQ_QUEUE_SIZE);

static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
static uint16_t   m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */

static ble_lns_loc_speed_t   m_sim_location_speed;          /**< Location and speed simulation. */
static ble_lns_pos_quality_t m_sim_position_quality;        /**< Position measurement quality simulation. */
static ble_lns_navigation_t  m_sim_navigation;              /**< Navigation data structure simulation. */

static const ble_lns_loc_speed_t initial_lns_location_speed =
{
    .instant_speed_present   = true,
    .total_distance_present  = true,
    .location_present        = true,
    .elevation_present       = true,
    .heading_present         = true,
    .rolling_time_present    = true,
    .utc_time_time_present   = true,
    .position_status         = BLE_LNS_POSITION_OK,
    .data_format             = BLE_LNS_SPEED_DISTANCE_FORMAT_2D,
    .elevation_source        = BLE_LNS_ELEV_SOURCE_POSITIONING_SYSTEM,
    .heading_source          = BLE_LNS_HEADING_SOURCE_COMPASS,
    .instant_speed           = 12,         // = 1.2 meter/second
    .total_distance          = 2356,       // = 2356 meters/second
    .latitude                = -103123567, // = -10.3123567 degrees
    .longitude               = 601234567,  // = 60.1234567 degrees
    .elevation               = 1350,       // = 13.5 meter
    .heading                 = 2123,       // = 21.23 degrees
    .rolling_time            = 1,          // = 1 second
    .utc_time                = {
                                 .year    = 2015,
                                 .month   = 7,
                                 .day     = 8,
                                 .hours   = 12,
                                 .minutes = 43,
                                 .seconds = 33
                               }
};

static const ble_lns_pos_quality_t initial_lns_pos_quality =
{
    .number_of_satellites_in_solution_present = true,
    .number_of_satellites_in_view_present     = true,
    .time_to_first_fix_present                = true,
    .ehpe_present                             = true,
    .evpe_present                             = true,
    .hdop_present                             = true,
    .vdop_present                             = true,
    .position_status                          = BLE_LNS_POSITION_OK,
    .number_of_satellites_in_solution         = 5,
    .number_of_satellites_in_view             = 6,
    .time_to_first_fix                        = 63,  // = 6.3 seconds
    .ehpe                                     = 100, // = 1 meter
    .evpe                                     = 123, // = 1.23 meter
    .hdop                                     = 123,
    .vdop                                     = 143
};

static const ble_lns_navigation_t initial_lns_navigation =
{
    .remaining_dist_present       = true,
    .remaining_vert_dist_present  = true,
    .eta_present                  = true,
    .position_status              = BLE_LNS_POSITION_OK,
    .heading_source               = BLE_LNS_HEADING_SOURCE_COMPASS,
    .navigation_indicator_type    = BLE_LNS_NAV_TO_WAYPOINT,
    .waypoint_reached             = false,
    .destination_reached          = false,
    .bearing                      = 1234,   // = 12.34 degrees
    .heading                      = 2123,   // = 21.23 degrees
    .remaining_distance           = 532576, // = 53257.6 meters
    .remaining_vert_distance      = 123,    // = 12.3 meters
    .eta                          = {
                                      .year    = 2015,
                                      .month   = 7,
                                      .day     = 8,
                                      .hours   = 16,
                                      .minutes = 43,
                                      .seconds = 33
                                   }
};

static uint32_t m_qwr_nrf_error = 0;

static void advertising_start(void);


/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    ble_gap_addr_t gap_addr;
    err_code = sd_ble_gap_addr_get(&gap_addr);
    APP_ERROR_CHECK(err_code);

    gap_addr.addr[5] = 0x0B;
    gap_addr.addr[4] = 0x0C;
    gap_addr.addr[3] = 0x04;
    gap_addr.addr[2] = 0xD1;
    gap_addr.addr[1] = 0x37;
    gap_addr.addr[0] = 0xB4;
#if ADV_FOR_IPHONE
    gap_addr.addr[0] = 0xB5; // B5 for iphone
#endif

    gap_addr.addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;

    err_code = sd_ble_gap_addr_set(&gap_addr);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_UNKNOWN);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */

static void nrf_qwr_error_handler(uint32_t nrf_error)
{
	m_qwr_nrf_error = nrf_error;
    APP_ERROR_HANDLER(nrf_error);
}

uint32_t nrf_qwr_error_get(void) {

	return m_qwr_nrf_error;
}

void nrf_qwr_error_reset(void)
{
	m_qwr_nrf_error = 0;
}


/**@brief Callback function for errors in the Location Navigation Service.
 *
 * @details This function will be called in case of an error in the Location Navigation Service.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 */
void lns_error_handler(uint32_t err_code)
{
    app_error_handler(DEAD_BEEF, 0, 0);
}

/**@brief Location Navigation event handler.
 *
 * @details This function will be called for all events of the Location Navigation Module that
 *          are passed to the application.
 *
 * @param[in]   p_evt   Event received from the Location Navigation Module.
 */
static void on_lns_evt(ble_lns_t const * p_lns, ble_lns_evt_t const * p_evt)
{
    switch (p_evt->evt_type)
    {
        case BLE_LNS_CTRLPT_EVT_INDICATION_ENABLED:
            NRF_LOG_INFO("Control Point: Indication enabled");
            break;

        case BLE_LNS_CTRLPT_EVT_INDICATION_DISABLED:
            NRF_LOG_INFO("Control Point: Indication disabled");
            break;

        case BLE_LNS_LOC_SPEED_EVT_NOTIFICATION_ENABLED:
            NRF_LOG_INFO("Location/Speed: Notification enabled");
            break;

        case BLE_LNS_LOC_SPEED_EVT_NOTIFICATION_DISABLED:
            NRF_LOG_INFO("Location/Speed: Notification disabled");
            break;

        case BLE_LNS_NAVIGATION_EVT_NOTIFICATION_ENABLED:
            NRF_LOG_INFO("Navigation: Notification enabled");
            break;

        case BLE_LNS_NAVIGATION_EVT_NOTIFICATION_DISABLED:
            NRF_LOG_INFO("Navigation: Notification disabled");
            break;

        default:
            break;
    }
}

ble_lncp_rsp_code_t on_ln_ctrlpt_evt(ble_lncp_t const * p_lncp, ble_lncp_evt_t const * p_evt)
{
    switch (p_evt->evt_type)
    {
        case LNCP_EVT_MASK_SET:
            NRF_LOG_INFO("LOC_SPEED_EVT: Feature mask set");
            break;

        case LNCP_EVT_TOTAL_DISTANCE_SET:
            NRF_LOG_INFO("LOC_SPEED_EVT: Set total distance: %d", p_evt->params.total_distance);
            break;

        case LNCP_EVT_ELEVATION_SET:
            NRF_LOG_INFO("LOC_SPEED_EVT: Set elevation: %d", p_evt->params.elevation);
            break;

        case LNCP_EVT_FIX_RATE_SET:
            NRF_LOG_INFO("POS_QUAL_EVT: Fix rate set to %d", p_evt->params.fix_rate);
            break;

        case LNCP_EVT_NAV_COMMAND:
            NRF_LOG_INFO("NAV_EVT: Navigation state changed to %d", p_evt->params.nav_cmd);
            break;

        case LNCP_EVT_ROUTE_SELECTED:
            NRF_LOG_INFO("NAV_EVT: Route selected %d", p_evt->params.selected_route);
            break;


        default:
            break;
    }

    return (LNCP_RSP_SUCCESS);
}


/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t * p_evt)
{

    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
    	app_handler__nus_data_handler(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
    }

}
/**@snippet [Handling the data received over BLE] */


/**@brief TODO Function for performing battery measurement and updating the Battery Level characteristic
 *        in Battery Service.
 */
static void battery_level_update(void)
{
    ret_code_t err_code;
    static uint8_t  battery_level = 66;

    err_code = ble_bas_battery_level_update(&m_bas, battery_level, BLE_CONN_HANDLE_ALL);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != NRF_ERROR_RESOURCES) &&
        (err_code != NRF_ERROR_BUSY) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
       )
    {
        APP_ERROR_HANDLER(err_code);
    }
}

#if SEC_PARAM_BOND
/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    pm_handler_on_pm_evt(p_evt);
    pm_handler_flash_clean(p_evt);

    switch (p_evt->evt_id)
    {
        case PM_EVT_PEERS_DELETE_SUCCEEDED:
            advertising_start();
            break;

        default:
            break;
    }
}
#endif


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t           err_code;
    ble_nus_init_t     nus_init;
    ble_dis_init_t     dis_init;
    ble_bas_init_t     bas_init;
    ble_lns_init_t     lns_init;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Device Information Service.
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char *)"Vincent Inc");

    dis_init.dis_char_rd_sec = SEC_OPEN;

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Battery Service.
    memset(&bas_init, 0, sizeof(bas_init));

    bas_init.evt_handler          = NULL;
    bas_init.support_notification = true;
    bas_init.p_report_ref         = NULL;
    bas_init.initial_batt_level   = 66;

    // Here the sec level for the Battery Service can be changed/increased.
    bas_init.bl_rd_sec        = SEC_OPEN;
    bas_init.bl_cccd_wr_sec   = SEC_OPEN;
    bas_init.bl_report_rd_sec = SEC_OPEN;

    err_code = ble_bas_init(&m_bas, &bas_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Location and Navigation Service.
    memset(&lns_init, 0, sizeof(lns_init));

    lns_init.evt_handler      = on_lns_evt;
    lns_init.lncp_evt_handler = on_ln_ctrlpt_evt;
    lns_init.error_handler    = lns_error_handler;
    lns_init.p_gatt_queue     = &m_ble_gatt_queue;

    lns_init.is_position_quality_present = true;
    lns_init.is_control_point_present    = true;
    lns_init.is_navigation_present       = true;

    lns_init.available_features     = BLE_LNS_FEATURE_INSTANT_SPEED_SUPPORTED                 |
                                      BLE_LNS_FEATURE_TOTAL_DISTANCE_SUPPORTED                |
                                      BLE_LNS_FEATURE_LOCATION_SUPPORTED                      |
                                      BLE_LNS_FEATURE_ELEVATION_SUPPORTED                     |
                                      BLE_LNS_FEATURE_HEADING_SUPPORTED                       |
                                      BLE_LNS_FEATURE_ROLLING_TIME_SUPPORTED                  |
                                      BLE_LNS_FEATURE_UTC_TIME_SUPPORTED                      |
                                      BLE_LNS_FEATURE_REMAINING_DISTANCE_SUPPORTED            |
                                      BLE_LNS_FEATURE_REMAINING_VERT_DISTANCE_SUPPORTED       |
                                      BLE_LNS_FEATURE_EST_TIME_OF_ARRIVAL_SUPPORTED           |
                                      BLE_LNS_FEATURE_NUM_SATS_IN_SOLUTION_SUPPORTED          |
                                      BLE_LNS_FEATURE_NUM_SATS_IN_VIEW_SUPPORTED              |
                                      BLE_LNS_FEATURE_TIME_TO_FIRST_FIX_SUPPORTED             |
                                      BLE_LNS_FEATURE_EST_HORZ_POS_ERROR_SUPPORTED            |
                                      BLE_LNS_FEATURE_EST_VERT_POS_ERROR_SUPPORTED            |
                                      BLE_LNS_FEATURE_HORZ_DILUTION_OF_PRECISION_SUPPORTED    |
                                      BLE_LNS_FEATURE_VERT_DILUTION_OF_PRECISION_SUPPORTED    |
                                      BLE_LNS_FEATURE_LOC_AND_SPEED_CONTENT_MASKING_SUPPORTED |
                                      BLE_LNS_FEATURE_FIX_RATE_SETTING_SUPPORTED              |
                                      BLE_LNS_FEATURE_ELEVATION_SETTING_SUPPORTED             |
                                      BLE_LNS_FEATURE_POSITION_STATUS_SUPPORTED;


    m_sim_location_speed   = initial_lns_location_speed;
    m_sim_position_quality = initial_lns_pos_quality;
    m_sim_navigation       = initial_lns_navigation;

    lns_init.p_location_speed   = &m_sim_location_speed;
    lns_init.p_position_quality = &m_sim_position_quality;
    lns_init.p_navigation       = &m_sim_navigation;

    lns_init.loc_nav_feature_security_req_read_perm  = SEC_OPEN;
    lns_init.loc_speed_security_req_cccd_write_perm  = SEC_OPEN;
    lns_init.position_quality_security_req_read_perm = SEC_OPEN;
    lns_init.navigation_security_req_cccd_write_perm = SEC_OPEN;
    lns_init.ctrl_point_security_req_write_perm      = SEC_OPEN;
    lns_init.ctrl_point_security_req_cccd_write_perm = SEC_OPEN;

    err_code = ble_lns_init(&m_lns, &lns_init);
    APP_ERROR_CHECK(err_code);

    ble_lns_route_t route1 = {.route_name = "Route one"};
    err_code = ble_lns_add_route(&m_lns, &route1);

    ble_lns_route_t route2 = {.route_name = "Route two"};
    err_code = ble_lns_add_route(&m_lns, &route2);
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    // TODO check
    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = m_lns.loc_speed_handles.cccd_handle;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:

            break;

        case BLE_ADV_EVT_IDLE:
            break;
        default:
            break;
    }
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected");

            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);

            app_handler__on_connected();
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected");
            // LED indication will be changed when advertising starts.
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

    	case BLE_GATTC_EVT_WRITE_CMD_TX_COMPLETE:
    		NRF_LOG_DEBUG("GATTC WRITE_CMD_TX Complete");
    		m_qwr_nrf_error = 0;
    		app_handler__signal();
    		break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            NRF_LOG_DEBUG("BLE_GAP_EVT_SEC_PARAMS_REQUEST");
            break;

        case BLE_GAP_EVT_AUTH_KEY_REQUEST:
            NRF_LOG_INFO("BLE_GAP_EVT_AUTH_KEY_REQUEST");
            break;

        case BLE_GAP_EVT_LESC_DHKEY_REQUEST:
            NRF_LOG_INFO("BLE_GAP_EVT_LESC_DHKEY_REQUEST");
            break;

         case BLE_GAP_EVT_AUTH_STATUS:
             NRF_LOG_INFO("BLE_GAP_EVT_AUTH_STATUS: status=0x%x bond=0x%x lv4: %d kdist_own:0x%x kdist_peer:0x%x",
                          p_ble_evt->evt.gap_evt.params.auth_status.auth_status,
                          p_ble_evt->evt.gap_evt.params.auth_status.bonded,
                          p_ble_evt->evt.gap_evt.params.auth_status.sm1_levels.lv4,
                          *((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_own),
                          *((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_peer));
            break;


        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
//    ret_code_t err_code;
//
//    err_code = nrf_sdh_enable_request();
//    APP_ERROR_CHECK(err_code);
//
//    // Configure the BLE stack using the default settings.
//    // Fetch the start address of the application RAM.
//    uint32_t ram_start = 0;
//    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
//    APP_ERROR_CHECK(err_code);
//
//    // Enable BLE stack.
//    err_code = nrf_sdh_ble_enable(&ram_start);
//    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}

#if SEC_PARAM_BOND
/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}
#endif

/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}


/**@snippet [Handling the data received over UART] */

uint32_t nus_data_send(uint8_t *p_data, uint16_t length) {

	return ble_nus_data_send(&m_nus, p_data, &length, m_conn_handle);
}

#define ANCS_UUID_SERVICE                   0xF431  //!< 16-bit service UUID for the Apple Notification Center Service.

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;
    ble_advdata_manuf_data_t manuf_data; //Variable to hold manufacturer specific data

    memset(&init, 0, sizeof(init));
    memset(&manuf_data, 0, sizeof(manuf_data));

    // manufacturer specific data
//    uint8_t data[]                = "Lezyne"; //Our data to advertise
//    manuf_data.company_identifier = 0x37B5; // reverse order
//    manuf_data.data.p_data        = data;
//    manuf_data.data.size          = sizeof(data);
//
////    init.advdata.p_manuf_specific_data = &manuf_data;
//    init.srdata.p_manuf_specific_data = &manuf_data;

#if ADV_FOR_IPHONE

    static ble_uuid_t _adv_uuids[1]; /**< Universally unique service identifiers. */

    /**@brief 128-bit service UUID for the Apple Notification Center Service. */
    ble_uuid128_t const ble_ancs_base_uuid128 =
    {
    		{
    				// 7905F431-B5CE-4E99-A40F-4B1E122D00D0
    				0xd0, 0x00, 0x2d, 0x12, 0x1e, 0x4b, 0x0f, 0xa4,
					0x99, 0x4e, 0xce, 0xb5, 0x31, 0xf4, 0x05, 0x79
    		}
    };
    uint8_t ancs_uuid_type;
    err_code = sd_ble_uuid_vs_add(&ble_ancs_base_uuid128, &ancs_uuid_type);
    APP_ERROR_CHECK(err_code);

    _adv_uuids[0].uuid = ANCS_UUID_SERVICE;
    _adv_uuids[0].type = ancs_uuid_type;

    init.srdata.uuids_solicited.uuid_cnt = sizeof(_adv_uuids) / sizeof(_adv_uuids[0]);
    init.srdata.uuids_solicited.p_uuids  = _adv_uuids;

#else

    static ble_uuid_t m_adv_uuids[1];

    m_adv_uuids[0].uuid = BLE_UUID_NUS_SERVICE;
    m_adv_uuids[0].type = m_nus.uuid_type;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

#endif

    init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = true;
    init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;
    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}


/**@brief Application main function.
 */
void ble_init(void)
{

    ble_stack_init();

    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
#if SEC_PARAM_BOND
    peer_manager_init();
#endif

	advertising_start();

    NRF_LOG_INFO("Go !");
    NRF_LOG_FLUSH();

    (void)task_create(app_handler__task, "lezyne_task", NULL);

}

