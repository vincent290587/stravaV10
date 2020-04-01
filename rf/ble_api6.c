

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf_sdm.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_db_discovery.h"
#include "ble_srv_common.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "nrf_pwr_mgmt.h"
#include "app_util.h"
#include "app_error.h"
#include "peer_manager.h"
#include "app_util.h"
#include "app_timer.h"
#include "fds.h"
#include "nrf_fstorage.h"
#include "ble_conn_state.h"
#include "nrf_ble_gq.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_scan.h"
#include "helper.h"
#include "ble_bas_c.h"
#include "ble_nus_c.h"
#include "ble_advdata.h"
#include "ble_lns_c.h"
#include "ble_komoot_c.h"
#include "ant.h"
#include "glasses.h"
#include "Model.h"
#include "Locator.h"
#include "sd_functions.h"
#include "ble_api_base.h"
#include "segger_wrapper.h"
#include "ring_buffer.h"
#include "Model.h"

#define BLE_DEVICE_NAME             "stravaV10"

#define APP_BLE_CONN_CFG_TAG        1                                   /**< A tag identifying the SoftDevice BLE configuration. */

#define APP_BLE_OBSERVER_PRIO       1                                   /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_SOC_OBSERVER_PRIO       1                                   /**< Applications' SoC observer priority. You shoulnd't need to modify this value. */


#define SEC_PARAM_BOND              1                                   /**< Perform bonding. */
#define SEC_PARAM_MITM              0                                   /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC              0                                   /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS          0                                   /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES   BLE_GAP_IO_CAPS_NONE                /**< No I/O capabilities. */
#define SEC_PARAM_OOB               0                                   /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE      7                                   /**< Minimum encryption key size in octets. */
#define SEC_PARAM_MAX_KEY_SIZE      16                                  /**< Maximum encryption key size in octets. */

#define SCAN_INTERVAL               0x00A0                              /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW                 0x0050                              /**< Determines scan window in units of 0.625 millisecond. */
#define SCAN_DURATION               0x0000                              /**< Duration of the scanning in units of 10 milliseconds. If set to 0x0000, scanning will continue until it is explicitly disabled. */
#define SCAN_DURATION_WITELIST      600                                /**< Duration of the scanning in units of 10 milliseconds. */

#define MIN_CONNECTION_INTERVAL     MSEC_TO_UNITS(7.5, UNIT_1_25_MS)    /**< Determines minimum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL     MSEC_TO_UNITS(30, UNIT_1_25_MS)     /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY               2                                   /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT         MSEC_TO_UNITS(4000, UNIT_10_MS)     /**< Determines supervision time-out in units of 10 millisecond. */

#define TARGET_UUID                 BLE_UUID_LOCATION_AND_NAVIGATION_SERVICE         /**< Target device name that application is looking for. */
#define TARGET_NAME                 "stravaAP"


typedef enum {
	eNusTransferStateIdle,
	eNusTransferStateInit,
	eNusTransferStateRun,
	eNusTransferStateWait,
	eNusTransferStateFinish
} eNusTransferState;


NRF_BLE_SCAN_DEF(m_scan);
BLE_NUS_C_DEF(m_ble_nus_c);
BLE_KOMOOT_C_DEF(m_ble_komoot_c);
BLE_LNS_C_DEF(m_ble_lns_c);                                             /**< Structure used to identify the heart rate client module. */
NRF_BLE_GATT_DEF(m_gatt);                                           /**< GATT module instance. */
BLE_DB_DISCOVERY_DEF(m_db_disc);                                    /**< DB discovery module instance. */
NRF_BLE_GQ_DEF(m_ble_gatt_queue,                                /**< BLE GATT Queue instance. */
        NRF_SDH_BLE_CENTRAL_LINK_COUNT,
        NRF_BLE_GQ_QUEUE_SIZE);

#define NUS_RB_SIZE      1024
RING_BUFFER_DEF(nus_rb1, NUS_RB_SIZE);

static bool                  m_retry_db_disc;              /**< Flag to keep track of whether the DB discovery should be retried. */
static uint16_t              m_pending_db_disc_conn = BLE_CONN_HANDLE_INVALID;  /**< Connection handle for which the DB discovery is retried. */

static volatile bool m_nus_cts = false;
static volatile bool m_connected = false;
static uint16_t m_nus_packet_nb = 0;
static uint16_t m_mtu_length = 20;

static eNusTransferState m_nus_xfer_state = eNusTransferStateIdle;


static void scan_start(void);



/**@brief Function for handling database discovery events.
 *
 * @details This function is callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
	ble_lns_c_on_db_disc_evt(&m_ble_lns_c, p_evt);
	ble_nus_c_on_db_disc_evt(&m_ble_nus_c, p_evt);
	ble_komoot_c_on_db_disc_evt(&m_ble_komoot_c, p_evt);
}

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
	ret_code_t            err_code;
	ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;

	W_SYSVIEW_RecordEnterISR();

	switch (p_ble_evt->header.evt_id)
	{
	case BLE_GAP_EVT_CONNECTED:
	{
		LOG_INFO("Connected.");
		m_connected = true;
		m_pending_db_disc_conn = p_ble_evt->evt.gap_evt.conn_handle;
		m_retry_db_disc = false;
		// Discover peer's services.
		err_code = ble_db_discovery_start(&m_db_disc, m_pending_db_disc_conn);
		if (err_code == NRF_ERROR_BUSY)
		{
			LOG_INFO("ble_db_discovery_start() returned busy, will retry later.");
			m_retry_db_disc = true;
		}
		else
		{
			APP_ERROR_CHECK(err_code);
		}

        const uint16_t mtu_desired = 200;

        LOG_INFO("mtu of %d requested", mtu_desired);
        err_code = nrf_ble_gatt_data_length_set(&m_gatt, p_ble_evt->evt.gap_evt.conn_handle, mtu_desired);      // UPDATE MTU HERE
        APP_ERROR_CHECK(err_code);

	} break;

	case BLE_GAP_EVT_DISCONNECTED:
	{
		LOG_INFO("Disconnected, reason 0x%x.",
				p_ble_evt->evt.gap_evt.params.disconnected.reason);

		m_connected = false;

		// Reset DB discovery structure.
		memset(&m_db_disc, 0 , sizeof (m_db_disc));

		scan_start();

	} break;

	case BLE_GAP_EVT_TIMEOUT:
	{
		if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN)
		{
			NRF_LOG_DEBUG("Scan timed out.");
			scan_start();
		}
		else if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
		{
			LOG_INFO("Connection Request timed out.");
		}
	} break;


	case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
		// Pairing not supported.
		err_code = sd_ble_gap_sec_params_reply(p_ble_evt->evt.gap_evt.conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
		APP_ERROR_CHECK(err_code);
		break;

	case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
		// Accepting parameters requested by peer.
		err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
				&p_gap_evt->params.conn_param_update_request.conn_params);
		APP_ERROR_CHECK(err_code);
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

	case BLE_GATTC_EVT_TIMEOUT:
		// Disconnect on GATT Client timeout event.
		LOG_INFO("GATT Client Timeout.");
		err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
				BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);
		break;

	case BLE_GATTS_EVT_TIMEOUT:
		// Disconnect on GATT Server timeout event.
		LOG_INFO("GATT Server Timeout.");
		err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
				BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);
		break;

	case BLE_GATTC_EVT_WRITE_CMD_TX_COMPLETE:
		LOG_DEBUG("GATTC WRITE_CMD_TX Complete %u", m_nus_cts);

		// clear to send more packets
		m_nus_cts = true;

		// unblock NUS servicing task
		if (m_tasks_id.peripherals_id != TASK_ID_INVALID) {
			w_task_delay_cancel(m_tasks_id.peripherals_id);
		}
		break;

	case BLE_GATTS_EVT_HVN_TX_COMPLETE:
		// unused here
		LOG_INFO("GATTC HVN_TX Complete");
		break;

	default:
		break;
	}


	W_SYSVIEW_RecordExitISR();

}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
	ret_code_t err_code;

	// Configure the BLE stack using the default settings.
	// Fetch the start address of the application RAM.
	uint32_t ram_start = 0;
	err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
	APP_ERROR_CHECK(err_code);

	// Enable BLE stack.
	err_code = nrf_sdh_ble_enable(&ram_start);
	APP_ERROR_CHECK(err_code);

	// Register handlers for BLE and SoC events.
	NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);

	// set name
	ble_gap_conn_sec_mode_t sec_mode; // Struct to store security parameters
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
	/*Get this device name*/
	uint8_t device_name[20];
	memset(device_name, 0, sizeof(device_name));
	memcpy(device_name, BLE_DEVICE_NAME, strlen(BLE_DEVICE_NAME));
	err_code = sd_ble_gap_device_name_set(&sec_mode, device_name, strlen(BLE_DEVICE_NAME));
}

/**@brief Heart Rate Collector Handler.
 */
static void lns_c_evt_handler(ble_lns_c_t * p_lns_c, ble_lns_c_evt_t * p_lns_c_evt)
{
	uint32_t err_code;

	NRF_LOG_DEBUG("LNS event: 0x%X\r\n", p_lns_c_evt->evt_type);

	switch (p_lns_c_evt->evt_type)
	{
	case BLE_LNS_C_EVT_DISCOVERY_COMPLETE:
		err_code = ble_lns_c_handles_assign(p_lns_c ,
				p_lns_c_evt->conn_handle,
				&p_lns_c_evt->params.peer_db);
		APP_ERROR_CHECK(err_code);

		// Initiate bonding.
		err_code = pm_conn_secure(p_lns_c_evt->conn_handle, false);
		if (err_code != NRF_ERROR_INVALID_STATE)
		{
			APP_ERROR_CHECK(err_code);
		}

		// LNS service discovered. Enable notification of LNS.
		err_code = ble_lns_c_pos_notif_enable(p_lns_c);
		APP_ERROR_CHECK(err_code);

		LOG_INFO("LNS service discovered.");
		break;

	case BLE_LNS_C_EVT_LNS_NOTIFICATION:
	{
		sLnsInfo lns_info;

		lns_info.lat = p_lns_c_evt->params.lns.lat;
		lns_info.lon = p_lns_c_evt->params.lns.lon;
		lns_info.ele = 0;
		lns_info.speed = 0;

		LOG_INFO("Latitude  = %ld", p_lns_c_evt->params.lns.lat);
		LOG_INFO("Longitude = %ld", p_lns_c_evt->params.lns.lon);

		LOG_INFO("Ele %ld", p_lns_c_evt->params.lns.ele);

		lns_info.secj = p_lns_c_evt->params.lns.utc_time.seconds;
		lns_info.secj += p_lns_c_evt->params.lns.utc_time.minutes * 60;
		lns_info.secj += p_lns_c_evt->params.lns.utc_time.hours * 3600;

		lns_info.date = p_lns_c_evt->params.lns.utc_time.year   % 100;
		lns_info.date += p_lns_c_evt->params.lns.utc_time.day   * 10000;
		lns_info.date += p_lns_c_evt->params.lns.utc_time.month * 100;

		if (p_lns_c_evt->params.lns.flags & ELE_PRESENT) {
			lns_info.ele = p_lns_c_evt->params.lns.ele;
		}

		if (p_lns_c_evt->params.lns.flags & INST_SPEED_PRESENT) {
			lns_info.speed = p_lns_c_evt->params.lns.inst_speed;
		}

		if (p_lns_c_evt->params.lns.flags & HEADING_PRESENT) {
			lns_info.heading = p_lns_c_evt->params.lns.heading;
		} else {
			lns_info.heading = -1;
		}

		locator_dispatch_lns_update(&lns_info);

		LOG_INFO("Sec jour = %d %d %d", p_lns_c_evt->params.lns.utc_time.hours,
				p_lns_c_evt->params.lns.utc_time.minutes,
				p_lns_c_evt->params.lns.utc_time.seconds);

		break;
	}

	default:
		break;
	}
}


/**@brief Battery level Collector Handler.
 */
static void nus_c_evt_handler(ble_nus_c_t * p_ble_nus_c, ble_nus_c_evt_t const * p_evt)
{
	ret_code_t err_code;

	switch (p_evt->evt_type)
	{
	case BLE_NUS_C_EVT_DISCOVERY_COMPLETE:
		LOG_INFO("Discovery complete.");
		err_code = ble_nus_c_handles_assign(p_ble_nus_c, p_evt->conn_handle, &p_evt->handles);
		APP_ERROR_CHECK(err_code);

		err_code = ble_nus_c_tx_notif_enable(p_ble_nus_c);
		APP_ERROR_CHECK(err_code);
		LOG_INFO("Connected to stravaAP");
		break;

	case BLE_NUS_C_EVT_NUS_TX_EVT:
		// handle received chars
		LOG_DEBUG("Received %u chars from BLE !", p_evt->data_len);

		{
			for (uint16_t i=0; i < p_evt->data_len; i++) {

				char c = p_evt->p_data[i];

				if (RING_BUFF_IS_NOT_FULL(nus_rb1)) {
					RING_BUFFER_ADD_ATOMIC(nus_rb1, c);
				} else {
					LOG_ERROR("NUS ring buffer full");

					// empty ring buffer
					RING_BUFF_EMPTY(nus_rb1);
				}

			}

			// unblock periph servicing task
			if (m_tasks_id.peripherals_id != TASK_ID_INVALID) {
				w_task_delay_cancel(m_tasks_id.peripherals_id);
			}
		}
		break;

	case BLE_NUS_C_EVT_DISCONNECTED:
		if (m_nus_xfer_state == eNusTransferStateRun) m_nus_xfer_state = eNusTransferStateFinish;
		break;
	}
}



/**@brief Heart Rate Collector Handler.
 */
static void komoot_c_evt_handler(ble_komoot_c_t * p_komoot_c, ble_komoot_c_evt_t * p_komoot_c_evt)
{
	uint32_t err_code;

	LOG_INFO("KOMOOT event: 0x%X\r\n", p_komoot_c_evt->evt_type);

	switch (p_komoot_c_evt->evt_type)
	{
	case BLE_KOMOOT_C_EVT_DISCOVERY_COMPLETE:
		err_code = ble_komoot_c_handles_assign(p_komoot_c ,
				p_komoot_c_evt->conn_handle,
				&p_komoot_c_evt->params.peer_db);
		APP_ERROR_CHECK(err_code);

		// Initiate bonding.
		err_code = pm_conn_secure(p_komoot_c_evt->conn_handle, false);
		if (err_code != NRF_ERROR_INVALID_STATE)
		{
			APP_ERROR_CHECK(err_code);
		}

		// service discovered. Enable notification
		err_code = ble_komoot_c_pos_notif_enable(p_komoot_c);
		APP_ERROR_CHECK(err_code);

		LOG_INFO("KOMOOT service discovered.");
		break;

	case BLE_KOMOOT_C_EVT_KOMOOT_NOTIFICATION:
	{
		uint32_t err_code = ble_komoot_c_nav_read(p_komoot_c);
		APP_ERROR_CHECK(err_code);
	}	break;

	case BLE_KOMOOT_C_EVT_KOMOOT_NAVIGATION:
	{
		m_komoot_nav.isUpdated = true;
		m_komoot_nav.direction = p_komoot_c_evt->params.komoot.direction;
		m_komoot_nav.distance = p_komoot_c_evt->params.komoot.distance;

		LOG_INFO("KOMOOT nav: direction %u", p_komoot_c_evt->params.komoot.direction);
	}   break;

	default:
		break;
	}

}


/**@brief Function for handling the LED Button Service client errors.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void _service_c_error_handler(uint32_t nrf_error)
{
	if (nrf_error == NRF_ERROR_RESOURCES) {

		LOG_DEBUG("NUS RESSSS %u", m_nus_packet_nb);
		m_nus_cts = false;
		return;
	}
	APP_ERROR_HANDLER(nrf_error);
}

/**
 * @brief Heart rate collector initialization.
 */
static void lns_c_init(void)
{
	ble_lns_c_init_t lns_c_init_obj;

	lns_c_init_obj.evt_handler = lns_c_evt_handler;
	lns_c_init_obj.p_gatt_queue  = &m_ble_gatt_queue;
	lns_c_init_obj.error_handler = _service_c_error_handler;

	uint32_t err_code = ble_lns_c_init(&m_ble_lns_c, &lns_c_init_obj);
	APP_ERROR_CHECK(err_code);
}


/**
 * @brief Heart rate collector initialization.
 */
static void komoot_c_init(void)
{
	ble_komoot_c_init_t komoot_c_init_obj;

	komoot_c_init_obj.evt_handler = komoot_c_evt_handler;
	komoot_c_init_obj.p_gatt_queue  = &m_ble_gatt_queue;
	komoot_c_init_obj.error_handler = _service_c_error_handler;

	uint32_t err_code = ble_komoot_c_init(&m_ble_komoot_c, &komoot_c_init_obj);
	APP_ERROR_CHECK(err_code);
}


/**
 * @brief NUS initialization.
 */
static void nus_c_init(void)
{
	ble_nus_c_init_t nus_c_init_obj;

	nus_c_init_obj.evt_handler = nus_c_evt_handler;
	nus_c_init_obj.p_gatt_queue  = &m_ble_gatt_queue;
	nus_c_init_obj.error_handler = _service_c_error_handler;

	uint32_t err_code = ble_nus_c_init(&m_ble_nus_c, &nus_c_init_obj);
	APP_ERROR_CHECK(err_code);
}


/**
 * @brief Database discovery collector initialization.
 */
static void db_discovery_init(void)
{
	ble_db_discovery_init_t db_init;

	memset(&db_init, 0, sizeof(db_init));

	db_init.evt_handler  = db_disc_handler;
	db_init.p_gatt_queue = &m_ble_gatt_queue;

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
		NRF_LOG_INFO("Filter match: %x", p_scan_evt->params.filter_match.filter_match.uuid_filter_match);
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
		NRF_LOG_INFO("Scan timed out.");
		scan_start();
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

	init_scan.connect_if_match = true;
	init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;

	/**@brief NUS UUID. */
	ble_uuid_t const m_lns_uuid =
	{
			.uuid = TARGET_UUID,
			.type = m_ble_lns_c.uuid_type
	};

	/**@brief NUS UUID. */
	ble_uuid_t const m_komoot_uuid =
	{
			.uuid = BLE_UUID_KOMOOT_SERVICE,
			.type = m_ble_komoot_c.uuid_type
	};

	err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_ble_scan_filter_set(&m_scan, SCAN_NAME_FILTER, TARGET_NAME);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_ble_scan_filter_set(&m_scan, SCAN_UUID_FILTER, &m_lns_uuid);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_ble_scan_filter_set(&m_scan, SCAN_UUID_FILTER, &m_komoot_uuid);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_ble_scan_filters_enable(&m_scan, NRF_BLE_SCAN_UUID_FILTER | NRF_BLE_SCAN_NAME_FILTER, false);
	APP_ERROR_CHECK(err_code);
}

/**@brief Function to start scanning.
 */
static void scan_start(void)
{
	ret_code_t ret;

	ret = nrf_ble_scan_start(&m_scan);
	APP_ERROR_CHECK(ret);
}

/**@brief GATT module event handler.
 */
static void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
	switch (p_evt->evt_id)
	{
	case NRF_BLE_GATT_EVT_ATT_MTU_UPDATED:
	{

		LOG_INFO("Desired MTU: central %u peripheral %u",
				p_gatt->att_mtu_desired_central,
				p_gatt->att_mtu_desired_periph);

		LOG_INFO("ATT MTU exchange completed. MTU set to %u bytes.",
				p_evt->params.att_mtu_effective);

		m_mtu_length = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;

	} break;

	case NRF_BLE_GATT_EVT_DATA_LENGTH_UPDATED:
	{
		LOG_INFO("Data length for connection 0x%x updated to %d.",
				p_evt->conn_handle,
				p_evt->params.data_length);
	} break;

	default:
		break;
	}

	if (m_retry_db_disc)
	{
		LOG_INFO("Retrying DB discovery.");

		m_retry_db_disc = false;

		// Discover peer's services.
		ret_code_t err_code;
		err_code = ble_db_discovery_start(&m_db_disc, m_pending_db_disc_conn);

		if (err_code == NRF_ERROR_BUSY)
		{
			LOG_INFO("ble_db_discovery_start() returned busy, will retry later.");
			m_retry_db_disc = true;
		}
		else
		{
			APP_ERROR_CHECK(err_code);
		}
	}
}


/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
	ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
	APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_central_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
	APP_ERROR_CHECK(err_code);
}

void ble_get_navigation(sKomootNavigation *nav) {

	ASSERT(nav);

	if (m_komoot_nav.isUpdated) memcpy(nav, &m_komoot_nav, sizeof(m_komoot_nav));

}

void ble_start_evt(eBleEventType evt) {

	switch (m_nus_xfer_state) {
	case eNusTransferStateIdle:
	{
		m_nus_xfer_state = eNusTransferStateInit;
	} break;
	default:
		LOG_WARNING("BLE is already doing a xfer");
		break;
	}

}

#ifdef BLE_STACK_SUPPORT_REQD
/**
 * Init BLE stack
 */
void ble_init(void)
{
	ble_stack_init();

	gatt_init();
	db_discovery_init();

	lns_c_init();
	nus_c_init();
	komoot_c_init();
	scan_init();

	// Start scanning for peripherals and initiate connection
	// with devices
	scan_start();
}


/**
 * Send the log file to a remote computer
 */
#include "sd_functions.h"
void ble_nus_tasks(void) {

	static char _buffer[BLE_NUS_MAX_DATA_LEN];
	static sCharArray m_nus_xfer_array = {
			.str = _buffer,
			.length = 0,
	};

	if (m_nus_xfer_state == eNusTransferStateIdle) {

		while (RING_BUFF_IS_NOT_EMPTY(nus_rb1)) {

			char c = RING_BUFF_GET_ELEM(nus_rb1);
			RING_BUFFER_POP(nus_rb1);

			model_input_virtual_uart(c);
		}

		return;
	}

	switch (m_nus_xfer_state) {
	case eNusTransferStateInit:
	{
		m_nus_packet_nb = 0;
		m_nus_cts = true;
		m_nus_xfer_array.length = 0;
		m_nus_xfer_state = eNusTransferStateRun;
	}
	break;

	case eNusTransferStateRun:
		if (!m_connected) {
			// problem or end of transfer
			m_nus_xfer_state = eNusTransferStateFinish;
		}
		break;

	case eNusTransferStateFinish:
	{
		int ret = sd_functions__stop_query();
		if (ret != 0) {
			LOG_WARNING("Query error stop %d", ret);
		} else {
			LOG_WARNING("NUS transfer completed :-)");
		}
		m_nus_xfer_state = eNusTransferStateIdle;
	} break;

	case eNusTransferStateIdle:
	default:
		break;
	}

	while (m_connected &&
			(m_nus_xfer_state == eNusTransferStateRun || (m_nus_xfer_state == eNusTransferStateFinish && m_nus_xfer_array.length)) &&
			m_nus_cts) {

		if (!m_nus_xfer_array.length) {

			// read whatever buffer
			int ret = sd_functions__run_query(0, &m_nus_xfer_array, m_mtu_length < sizeof(_buffer) ? m_mtu_length : sizeof(_buffer));
			if (ret) {

				LOG_INFO("sd_functions__run_query error %d", ret);
				return;
			}

			LOG_INFO("sd_functions__run_query sending %u bytes", m_nus_xfer_array.length);
		}

		if (!m_nus_xfer_array.str || !m_nus_xfer_array.length) {
			LOG_INFO("Log file end, %u packets sent", m_nus_packet_nb);
			// problem or end of transfer
			m_nus_xfer_state = eNusTransferStateFinish;
			return;
		}

		uint32_t err_code = ble_nus_c_string_send(&m_ble_nus_c, (uint8_t *)m_nus_xfer_array.str, m_nus_xfer_array.length);

		switch (err_code) {
		case NRF_ERROR_BUSY:
			LOG_WARNING("NUS BUSY");
			return;
			break;

		case NRF_ERROR_RESOURCES:
			LOG_DEBUG("NUS RESSSS %u", m_nus_packet_nb);
			m_nus_cts = false;
			break;

		case NRF_ERROR_TIMEOUT:
			LOG_WARNING("NUS timeout", err_code);
			return;
			break;

		case NRF_SUCCESS:
		{
			if (m_nus_cts) {
				LOG_DEBUG("Packet %u sent size %u", m_nus_packet_nb, m_nus_xfer_array.length);

				m_nus_packet_nb++;
				m_nus_xfer_array.length = 0;
			}
		} break;

		default:
		{
			LOG_WARNING("NUS unknown error: 0x%X MTU %u / %u", err_code, m_nus_xfer_array.length, m_mtu_length);
			return;
		} break;
		}

	}

}


#endif


