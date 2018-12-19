
#include "sdk_common.h"

#if 1 || NRF_MODULE_ENABLED(BLE_KOMOOT_C)
#include "app_util.h"
#include "app_util_bds.h"
#include "ble_komoot_c.h"
#include "ble_db_discovery.h"
#include "ble_types.h"
#include "ble_srv_common.h"
#include "ble_gattc.h"

#include "segger_wrapper.h"

#define TX_BUFFER_MASK         0x07                  /**< TX Buffer mask, must be a mask of continuous zeroes, followed by continuous sequence of ones: 000...111. */
#define TX_BUFFER_SIZE         (TX_BUFFER_MASK + 1)  /**< Size of send buffer, which is 1 higher than the mask. */

#define WRITE_MESSAGE_LENGTH   BLE_CCCD_VALUE_LEN    /**< Length of the write message for CCCD. */
#define WRITE_MESSAGE_LENGTH   BLE_CCCD_VALUE_LEN    /**< Length of the write message for CCCD. */

typedef enum
{
	READ_REQ,  /**< Type identifying that this tx_message is a read request. */
	WRITE_REQ  /**< Type identifying that this tx_message is a write request. */
} tx_request_t;

/**@brief Structure for writing a message to the peer, i.e. CCCD.
 */
typedef struct
{
	uint8_t                  gattc_value[WRITE_MESSAGE_LENGTH];  /**< The message to write. */
	ble_gattc_write_params_t gattc_params;                       /**< GATTC parameters for this message. */
} write_params_t;

/**@brief Structure for holding data to be transmitted to the connected central.
 */
typedef struct
{
	uint16_t     conn_handle;  /**< Connection handle to be used when transmitting this message. */
	tx_request_t type;         /**< Type of this message, i.e. read or write message. */
	union
	{
		uint16_t       read_handle;  /**< Read request message. */
		write_params_t write_req;    /**< Write request message. */
	} req;
} tx_message_t;


static tx_message_t  m_tx_buffer[TX_BUFFER_SIZE];  /**< Transmit buffer for messages to be transmitted to the central. */
static uint32_t      m_tx_insert_index = 0;        /**< Current index in the transmit buffer where the next message should be inserted. */
static uint32_t      m_tx_index = 0;               /**< Current index in the transmit buffer from where the next message to be transmitted resides. */


static __INLINE uint8_t bds_int32_decode(const uint8_t * p_encoded_data,
		int32_t       * p_decoded_val)
{
	uint32_t tmp = 0;
	uint8_t retval = bds_uint32_decode(4, p_encoded_data, &tmp);
	*p_decoded_val = (int32_t)tmp;
	return retval;
}


/**@brief Function for passing any pending request from the buffer to the stack.
 */
static void tx_buffer_process(void)
{
	if (m_tx_index != m_tx_insert_index)
	{
		uint32_t err_code;

		if (m_tx_buffer[m_tx_index].type == READ_REQ)
		{
			err_code = sd_ble_gattc_read(m_tx_buffer[m_tx_index].conn_handle,
					m_tx_buffer[m_tx_index].req.read_handle, 0);
		}
		else
		{
			err_code = sd_ble_gattc_write(m_tx_buffer[m_tx_index].conn_handle,
					&m_tx_buffer[m_tx_index].req.write_req.gattc_params);
		}
		if (err_code == NRF_SUCCESS)
		{
			LOG_INFO("SD Read/Write (%u) API returns Success..\r\n",
					m_tx_buffer[m_tx_index].type);
			m_tx_index++;
			m_tx_index &= TX_BUFFER_MASK;
		}
		else
		{
			LOG_INFO("SD Read/Write (%u) API returns error. This message sending will be "
					"attempted again..\r\n",
					m_tx_buffer[m_tx_index].type);
		}
	}
}


/**@brief     Function for handling write response events.
 *
 * @param[in] p_ble_komoot_c Pointer to the Client structure.
 * @param[in] p_ble_evt   Pointer to the BLE event received.
 */
static void on_write_rsp(ble_komoot_c_t * p_ble_komoot_c, const ble_evt_t * p_ble_evt)
{
	// Check if the event if on the link for this instance
	if (p_ble_komoot_c->conn_handle != p_ble_evt->evt.gattc_evt.conn_handle)
	{
		return;
	}

	// Check if there is any message to be sent across to the peer and send it.
	tx_buffer_process();
}

static uint32_t m_identifier = 0;

/**@brief Function for creating a message for writing to the CCCD.
 */
static uint32_t cccd_request(uint16_t conn_handle, uint16_t handle_cccd)
{
    tx_message_t * p_msg;
//    uint16_t       cccd_val = enable ? BLE_GATT_HVX_NOTIFICATION : 0;

    LOG_INFO("Requesting CCCD for new identifier");

    p_msg              = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index &= TX_BUFFER_MASK;

    uint8_t msg[4] = {0};
    uint32_encode(m_identifier, msg);

    p_msg->req.write_req.gattc_params.handle   = handle_cccd;
    p_msg->req.write_req.gattc_params.len      = sizeof(msg);
    p_msg->req.write_req.gattc_params.p_value  = msg;
    p_msg->req.write_req.gattc_params.offset   = 0;
    p_msg->req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;
//    p_msg->req.write_req.gattc_value[0]        = LSB_16(cccd_val);
//    p_msg->req.write_req.gattc_value[1]        = MSB_16(cccd_val);
    p_msg->conn_handle                         = conn_handle;
    p_msg->type                                = READ_REQ;

    tx_buffer_process();
    return NRF_SUCCESS;
}


/**@brief     Function for handling Handle Value Notification received from the SoftDevice.
 *
 * @details   This function will uses the Handle Value Notification received from the SoftDevice
 *            and checks if it is a notification of the heart rate measurement from the peer. If
 *            it is, this function will decode the heart rate measurement and send it to the
 *            application.
 *
 * @param[in] p_ble_komoot_c Pointer to the Heart Rate Client structure.
 * @param[in] p_ble_evt   Pointer to the BLE event received.
 */
static void on_hvx(ble_komoot_c_t * p_ble_komoot_c, const ble_evt_t * p_ble_evt)
{
	// Check if the event is on the link for this instance
	if (p_ble_komoot_c->conn_handle != p_ble_evt->evt.gattc_evt.conn_handle)
	{
		LOG_INFO("received HVX on link 0x%x, not associated to this instance, ignore\r\n",
				p_ble_evt->evt.gattc_evt.conn_handle);
		return;
	}
	LOG_INFO("received HVX on handle 0x%x, komoot_handle 0x%x\r\n",
			p_ble_evt->evt.gattc_evt.params.hvx.handle,
			p_ble_komoot_c->peer_komoot_db.komoot_handle);

	// Check if this is a notification.
	if (p_ble_evt->evt.gattc_evt.params.hvx.handle == p_ble_komoot_c->peer_komoot_db.komoot_handle)
	{
		ble_komoot_c_evt_t ble_komoot_c_evt;
		uint32_t        index = 0;

		ble_komoot_c_evt.evt_type                    = BLE_KOMOOT_C_EVT_KOMOOT_NOTIFICATION;
		ble_komoot_c_evt.conn_handle                 = p_ble_komoot_c->conn_handle;

		LOG_INFO("Read HVX: ");
		for (int i=0; i < p_ble_evt->evt.gattc_evt.params.hvx.len; i++) {
			LOG_INFO("0x%02X ", p_ble_evt->evt.gattc_evt.params.hvx.data[i]);
		}

		m_identifier = uint32_decode(&(p_ble_evt->evt.gattc_evt.params.hvx.data[index]));  //lint !e415 suppress Lint Warning 415: Likely access out of bond
		index += sizeof(uint32_t);

		if (p_ble_evt->evt.gattc_evt.params.hvx.len < 5) {
			cccd_request(p_ble_komoot_c->conn_handle, p_ble_komoot_c->peer_komoot_db.komoot_cccd_handle);
		} else {

			ble_komoot_c_evt.params.komoot.identifier = m_identifier;

			ble_komoot_c_evt.params.komoot.direction = p_ble_evt->evt.gattc_evt.params.hvx.data[index++];

			ble_komoot_c_evt.params.komoot.distance = uint32_decode(&(p_ble_evt->evt.gattc_evt.params.hvx.data[index]));  //lint !e415 suppress Lint Warning 415: Likely access out of bond
			index += sizeof(uint32_t);

			p_ble_komoot_c->evt_handler(p_ble_komoot_c, &ble_komoot_c_evt);

		}
	}
}


/**@brief     Function for handling Disconnected event received from the SoftDevice.
 *
 * @details   This function check if the disconnect event is happening on the link
 *            associated with the current instance of the module, if so it will set its
 *            conn_handle to invalid.
 *
 * @param[in] p_ble_komoot_c Pointer to the Heart Rate Client structure.
 * @param[in] p_ble_evt   Pointer to the BLE event received.
 */
static void on_disconnected(ble_komoot_c_t * p_ble_komoot_c, const ble_evt_t * p_ble_evt)
{
	if (p_ble_komoot_c->conn_handle == p_ble_evt->evt.gap_evt.conn_handle)
	{
		p_ble_komoot_c->conn_handle                 = BLE_CONN_HANDLE_INVALID;
		p_ble_komoot_c->peer_komoot_db.komoot_cccd_handle = BLE_GATT_HANDLE_INVALID;
		p_ble_komoot_c->peer_komoot_db.komoot_handle      = BLE_GATT_HANDLE_INVALID;
	}
}


void ble_komoot_c_on_db_disc_evt(ble_komoot_c_t * p_ble_komoot_c, const ble_db_discovery_evt_t * p_evt)
{
	// Check if the Rate Service was discovered.
	if (p_evt->params.discovered_db.srv_uuid.uuid == BLE_UUID_KOMOOT_SERVICE &&
			p_evt->params.discovered_db.srv_uuid.type == p_ble_komoot_c->uuid_type)
	{
		// Find the CCCD Handle of the Heart Rate Measurement characteristic.
		uint32_t i;

		ble_komoot_c_evt_t evt;

		NRF_LOG_DEBUG("Database Discovery handler called with event 0x%x\r\n", p_evt->evt_type);

		evt.conn_handle = p_evt->conn_handle;
		evt.evt_type    = BLE_KOMOOT_C_EVT_DISCOVERY_FAILED;

		if (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE) {

			evt.evt_type    = BLE_KOMOOT_C_EVT_DISCOVERY_COMPLETE;

			LOG_INFO("Database Discovery complete: %u CHAR found\r\n",
					p_evt->params.discovered_db.char_count);

			for (i = 0; i < p_evt->params.discovered_db.char_count; i++)
			{

				if (p_evt->params.discovered_db.charateristics[i].characteristic.uuid.uuid ==
						BLE_UUID_KOMOOT_CHAR)
				{
					// Found KOMOOT characteristic. Store CCCD handle and break.
					evt.params.peer_db.komoot_cccd_handle =
							p_evt->params.discovered_db.charateristics[i].cccd_handle;
					evt.params.peer_db.komoot_handle =
							p_evt->params.discovered_db.charateristics[i].characteristic.handle_value;

					LOG_INFO("Storing CCCD handle.\r\n");
				}

				LOG_INFO("Discovered CHAR: 0x%04X\r\n", p_evt->params.discovered_db.charateristics[i].characteristic.uuid.uuid);
				//NRF_LOG_FLUSH();
			}

			LOG_INFO("KOMOOT Service discovered at peer.\r\n");
			//If the instance has been assigned prior to db_discovery, assign the db_handles
			if (p_ble_komoot_c->conn_handle != BLE_CONN_HANDLE_INVALID)
			{
				if ((p_ble_komoot_c->peer_komoot_db.komoot_cccd_handle == BLE_GATT_HANDLE_INVALID)&&
						(p_ble_komoot_c->peer_komoot_db.komoot_handle == BLE_GATT_HANDLE_INVALID))
				{
					p_ble_komoot_c->peer_komoot_db = evt.params.peer_db;
				}
			}

		} else {
			LOG_INFO("Database Discovery failed\r\n");
		}

		p_ble_komoot_c->evt_handler(p_ble_komoot_c, &evt);
	}
}


uint32_t ble_komoot_c_init(ble_komoot_c_t * p_ble_komoot_c, ble_komoot_c_init_t * p_ble_komoot_c_init)
{
	VERIFY_PARAM_NOT_NULL(p_ble_komoot_c);
	VERIFY_PARAM_NOT_NULL(p_ble_komoot_c_init);

	ble_uuid_t komoot_uuid;
	ble_uuid128_t base_uuid_service = {BLE_BASE_UUID_KOMOOT_SERVICE};

	// Assign UUID types.
	ret_code_t err_code;
	err_code = sd_ble_uuid_vs_add(&base_uuid_service, &p_ble_komoot_c->uuid_type);
	VERIFY_SUCCESS(err_code);

	komoot_uuid.uuid = BLE_UUID_KOMOOT_SERVICE;
	komoot_uuid.type = p_ble_komoot_c->uuid_type;

	p_ble_komoot_c->evt_handler                 = p_ble_komoot_c_init->evt_handler;
	p_ble_komoot_c->conn_handle                 = BLE_CONN_HANDLE_INVALID;
	p_ble_komoot_c->peer_komoot_db.komoot_cccd_handle = BLE_GATT_HANDLE_INVALID;
	p_ble_komoot_c->peer_komoot_db.komoot_handle      = BLE_GATT_HANDLE_INVALID;

	return ble_db_discovery_evt_register(&komoot_uuid);
}


void ble_komoot_c_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
	ble_komoot_c_t * p_ble_komoot_c = (ble_komoot_c_t *)p_context;

	if ((p_ble_komoot_c == NULL) || (p_ble_evt == NULL))
	{
		return;
	}

	switch (p_ble_evt->header.evt_id)
	{
	case BLE_GATTC_EVT_HVX:
		on_hvx(p_ble_komoot_c, p_ble_evt);
		break;

	case BLE_GATTC_EVT_WRITE_RSP:
		on_write_rsp(p_ble_komoot_c, p_ble_evt);
		break;

	case BLE_GAP_EVT_DISCONNECTED:
		on_disconnected(p_ble_komoot_c, p_ble_evt);
		break;

	default:
		break;
	}
}


/**@brief Function for creating a message for writing to the CCCD.
 */
static uint32_t cccd_configure(uint16_t conn_handle, uint16_t handle_cccd, bool enable)
{
	LOG_INFO("Configuring CCCD. CCCD Handle = %d, Connection Handle = %d\r\n",
			handle_cccd, conn_handle);

    tx_message_t * p_msg;
    uint16_t       cccd_val = enable ? BLE_GATT_HVX_NOTIFICATION : 0;

    p_msg              = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index &= TX_BUFFER_MASK;

    uint8_t msg[WRITE_MESSAGE_LENGTH];
    msg[0] = LSB_16(cccd_val);
    msg[1] = MSB_16(cccd_val);

    p_msg->req.write_req.gattc_params.handle   = handle_cccd;
    p_msg->req.write_req.gattc_params.len      = WRITE_MESSAGE_LENGTH;
    p_msg->req.write_req.gattc_params.p_value  = msg;
    p_msg->req.write_req.gattc_params.offset   = 0;
    p_msg->req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;
    p_msg->req.write_req.gattc_value[0]        = LSB_16(cccd_val);
    p_msg->req.write_req.gattc_value[1]        = MSB_16(cccd_val);
    p_msg->conn_handle                         = conn_handle;
    p_msg->type                                = WRITE_REQ;

    tx_buffer_process();
    return NRF_SUCCESS;
}


uint32_t ble_komoot_c_pos_notif_enable(ble_komoot_c_t * p_ble_komoot_c)
{
	VERIFY_PARAM_NOT_NULL(p_ble_komoot_c);

    if ( (p_ble_komoot_c->conn_handle == BLE_CONN_HANDLE_INVALID)
       ||(p_ble_komoot_c->peer_komoot_db.komoot_cccd_handle == BLE_GATT_HANDLE_INVALID)
       )
    {
        return NRF_ERROR_INVALID_STATE;
    }
	return cccd_configure(p_ble_komoot_c->conn_handle, p_ble_komoot_c->peer_komoot_db.komoot_cccd_handle, true);
}


uint32_t ble_komoot_c_handles_assign(ble_komoot_c_t * p_ble_komoot_c,
		uint16_t conn_handle,
		const komoot_db_t * p_peer_komoot_handles)
{
	VERIFY_PARAM_NOT_NULL(p_ble_komoot_c);

	p_ble_komoot_c->conn_handle = conn_handle;
	if (p_peer_komoot_handles != NULL)
	{
		p_ble_komoot_c->peer_komoot_db = *p_peer_komoot_handles;
	}
	return NRF_SUCCESS;
}
/** @}
 *  @endcond
 */
#endif // NRF_MODULE_ENABLED(BLE_KOMOOT_C)
