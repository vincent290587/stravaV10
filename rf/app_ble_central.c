/*
 * app_ble_central.c
 *
 *  Created on: 27 mai 2020
 *      Author: vgol
 */

#include "g_structs.h"

#include "nordic_common.h"
#include "app_error.h"
#include "bsp_btn_ble.h"
#include "ble_db_discovery.h"
#include "nrf_ble_scan.h"
#include "nrf_ble_gatt.h"
#include "ble_cp_c.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "ble_api_base.h"

#define APP_BLE_CONN_CFG_TAG    1                                       /**< Tag that refers to the BLE stack configuration set with @ref sd_ble_cfg_set. The default tag is @ref BLE_CONN_CFG_TAG_DEFAULT. */
#define APP_BLE_OBSERVER_PRIO   3                                       /**< BLE observer priority of the application. There is no need to modify this value. */

#define SCAN_DURATION               MSEC_TO_UNITS(3*60000, UNIT_10_MS)     /**< Duration of the scanning in units of 10 milliseconds. If set to 0x0000, scanning will continue until it is explicitly disabled. */

BLE_CP_C_DEF(m_ble_cp_c);
BLE_DB_DISCOVERY_DEF(m_db_disc);                                        /**< Database discovery module instance. */
NRF_BLE_GQ_DEF(m_ble_gatt_queue_c,                                        /**< BLE GATT Queue instance. */
               NRF_SDH_BLE_CENTRAL_LINK_COUNT,
               NRF_BLE_GQ_QUEUE_SIZE);
NRF_BLE_SCAN_DEF(m_scan);                                               /**< Scanning Module instance. */

/**< Scan parameters requested for scanning and connection. */
static ble_gap_scan_params_t const m_scan_param =
{
		.active        = 0x01,
		.interval      = 320,  /**< Determines scan interval in units of 0.625 millisecond. */
		.window        = 8,    /**< Determines scan window in units of 0.625 millisecond. */
		.filter_policy = BLE_GAP_SCAN_FP_ACCEPT_ALL,
		.timeout       = SCAN_DURATION,
		.scan_phys     = BLE_GAP_PHY_AUTO,
};

sPowerVector  powerVector;




/**@brief Function for handling the LED Button Service client errors.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void _service_c_error_handler(uint32_t nrf_error)
{

	APP_ERROR_HANDLER(nrf_error);
}

/**@brief Heart Rate Collector Handler.
 */
static void cp_c_evt_handler(ble_cp_c_t * p_cp_c, ble_cp_c_evt_t * p_cp_c_evt)
{
	ret_code_t err_code;

	switch (p_cp_c_evt->evt_type)
	{
	case BLE_CP_C_EVT_DISCOVERY_COMPLETE:
	{
		NRF_LOG_DEBUG("FTMS service discovered.");

		err_code = ble_cp_c_handles_assign(p_cp_c,
				p_cp_c_evt->conn_handle,
				&p_cp_c_evt->params.char_handles);
		APP_ERROR_CHECK(err_code);

		// FTMS discovered. Enable notifications
		err_code = ble_cp_c_pv_notif_enable(p_cp_c);
		APP_ERROR_CHECK(err_code);
	} break;

	case BLE_CP_C_EVT_VECTOR_RECV:
	{
		NRF_LOG_INFO("Crank revs: %u", p_cp_c_evt->params.vector_evt.cumul_crank_rev);
		NRF_LOG_INFO("Array pos : %u", p_cp_c_evt->params.vector_evt.array_size);

		powerVector.array_size      = p_cp_c_evt->params.vector_evt.array_size;
		powerVector.cumul_crank_rev = p_cp_c_evt->params.vector_evt.cumul_crank_rev;
		powerVector.first_crank_angle = p_cp_c_evt->params.vector_evt.first_crank_angle;
		powerVector.last_crank_evt  = p_cp_c_evt->params.vector_evt.last_crank_evt;
		memcpy(powerVector.inst_torque_mag_array, p_cp_c_evt->params.vector_evt.inst_torque_mag_array, sizeof(powerVector.inst_torque_mag_array));

	} break;

	case BLE_CP_C_EVT_POWER_RECV:
	{
		NRF_LOG_DEBUG("BLE_CP_C_EVT_POWER_RECV");

		powerVector.inst_power = p_cp_c_evt->params.power_meas.inst_power;

	} break;

	default:
		break;
	}
}

/**
 * @brief Heart rate collector initialization.
 */
static void cp_c_init(void)
{
	ble_cp_c_init_t cp_c_init_obj;

	cp_c_init_obj.evt_handler   = cp_c_evt_handler;
	cp_c_init_obj.p_gatt_queue  = &m_ble_gatt_queue_c;
	cp_c_init_obj.error_handler = _service_c_error_handler;

	memset(&m_ble_cp_c, 0, sizeof(m_ble_cp_c));

	ret_code_t err_code = ble_cp_c_init(&m_ble_cp_c, &cp_c_init_obj);
	APP_ERROR_CHECK(err_code);
}

static void services_c_init(void) {

	cp_c_init();
}

/**@brief Function for handling database discovery events.
 *
 * @details This function is a callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function forwards the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{

	ble_cp_c_on_db_disc_evt(&m_ble_cp_c, p_evt);
}


/** @brief Function for initializing the database discovery module. */
static void db_discovery_init(void)
{
    ble_db_discovery_init_t db_init;

    memset(&db_init, 0, sizeof(ble_db_discovery_init_t));

    db_init.evt_handler  = db_disc_handler;
    db_init.p_gatt_queue = &m_ble_gatt_queue_c;

    ret_code_t err_code = ble_db_discovery_init(&db_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Scanning Module events.
 */
static void scan_evt_handler(scan_evt_t const * p_scan_evt)
{
	ret_code_t err_code;

	switch(p_scan_evt->scan_evt_id)
	{
	case NRF_BLE_SCAN_EVT_FILTER_MATCH:
	{
		NRF_LOG_DEBUG("Filter match: %x", p_scan_evt->params.filter_match.filter_match.uuid_filter_match);
	} break;

	case NRF_BLE_SCAN_EVT_CONNECTING_ERROR:
	{
		err_code = p_scan_evt->params.connecting_err.err_code;
		APP_ERROR_CHECK(err_code);
	} break;

	case NRF_BLE_SCAN_EVT_CONNECTED:
	{
		ble_gap_evt_connected_t const * p_connected =
				p_scan_evt->params.connected.p_connected;
		// Scan is automatically stopped by the connection.
		NRF_LOG_DEBUG("Connecting to target %02x%02x%02x%02x%02x%02x",
				p_connected->peer_addr.addr[0],
				p_connected->peer_addr.addr[1],
				p_connected->peer_addr.addr[2],
				p_connected->peer_addr.addr[3],
				p_connected->peer_addr.addr[4],
				p_connected->peer_addr.addr[5]
		);
	} break;

	case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT:
	{
		NRF_LOG_INFO("BLE Scan timed out.");
		//scan_start();
	} break;

	default:
		break;
	}
}

/**@brief Function to start scanning.
 */
static void scan_init(void)
{
	ret_code_t          err_code;
	nrf_ble_scan_init_t init_scan;

	memset(&init_scan, 0, sizeof(init_scan));

	init_scan.p_scan_param     = &m_scan_param;
	init_scan.connect_if_match = true;
	init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;

	ble_uuid_t const m_cp_uuid =
	{
			.uuid = BLE_UUID_CYCLING_POWER_SERVICE,
			.type = BLE_UUID_TYPE_BLE,
	};

	err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_ble_scan_filter_set(&m_scan, SCAN_UUID_FILTER, &m_cp_uuid);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_ble_scan_filters_enable(&m_scan, NRF_BLE_SCAN_UUID_FILTER, false);
	APP_ERROR_CHECK(err_code);
}

/**@brief Function to start scanning.
 */
static void scan_start(void)
{

	if (ble_conn_state_central_conn_count() < NRF_SDH_BLE_CENTRAL_LINK_COUNT)
	{
		ret_code_t ret;
		ret = nrf_ble_scan_start(&m_scan);
		APP_ERROR_CHECK(ret);
	}

}

void app_ble_central_init(void)
{
//    m_on_data_received = on_data_received;
    db_discovery_init();
    services_c_init();
    scan_init();
    scan_start();
}
