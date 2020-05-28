/*
 * app_packets_handler.c
 *
 *  Created on: 27 mai 2020
 *      Author: vgol
 */


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
#include "millis.h"
#include "ble_nus.h"
#include "ble_api_base.h"
#include "nrf_queue.h"
#include "g_structs.h"
#include "sd_functions.h"

#include "task_manager.h"

#include "l_protocol.h"
#include "inseg_handler.h"
#include "app_packets_handler.h"

#include "segger_wrapper.h"

#define NUS_LONG_PACKETS_SIZE     20


typedef struct {
	uint8_t nus_data[BLE_NUS_STD_DATA_LEN];
	uint16_t data_length;
} sNusPacket;

typedef struct {
	uint8_t  nus_data[NUS_LONG_PACKETS_SIZE];
	uint16_t data_length;
} sNusLongPacket;

task_id_t m_rx_task_id = TASK_ID_INVALID;

NRF_QUEUE_DEF(sNusLongPacket, m_nus_tx_queue, 4, NRF_QUEUE_MODE_NO_OVERFLOW);
NRF_QUEUE_DEF(sNusPacket, m_nus_rx_queue, 20, NRF_QUEUE_MODE_NO_OVERFLOW);


static uint8_t  m_fit_up = 0;
static uint16_t m_nb_segs = 0;

// 631065600uL is GMT: Sunday 31 December 1989 00:00:00

// left part is unix timestamp in SECONDS
// 958 647 620uL + 631 065 600uL = 1 589 713 220 ===> GMT: Sunday 17 May 2020 11:00:20 :  GOOD !
//static uint32_t m_fake_name = 958647621uL + 1; // FIT timestamp

// 959358152 <===> 392EA4C7 from device

//static uint32_t m_fake_name = 1590002145uL - 631 065 600uL;

static uint8_t _process_tx(void);

static void _encode_uint32_little(uint32_t value, uint8_t p_data[]) {

	uint8_t index = 0;

	p_data[index++] = value & 0xFF;
	p_data[index++] = (value >> 8) & 0xFF;
	p_data[index++] = (value >> 16) & 0xFF;
	p_data[index++] = (value >> 24) & 0xFF;
}

static uint32_t _decode_uint32_little(uint8_t const *p_data) {

	uint32_t res = 0;

	res = p_data[0];
	res |= p_data[1] << 8;
	res |= p_data[2] << 16;
	res |= p_data[3] << 24;

	return res;
}

static uint16_t _decode_uint16_little(uint8_t const *p_data) {

	uint16_t res = 0;

	res = p_data[0];
	res |= p_data[1] << 8;

	return res;
}

static void _queue_nus(uint8_t const *p_data, uint16_t length) {

	sNusLongPacket data_array;

	memcpy(&data_array.nus_data, p_data, length);
	data_array.data_length = length;

	NRF_LOG_HEXDUMP_DEBUG(p_data, length);

	//NRF_LOG_INFO("Queuing packet %u", p_data[0]);

	while (nrf_queue_is_full(&m_nus_tx_queue)) {
		_process_tx();
	}

	ret_code_t err_code;
	err_code = nrf_queue_push(&m_nus_tx_queue, &data_array);
	APP_ERROR_CHECK(err_code);

	task_delay_cancel(m_rx_task_id);
}

static void _handle_send_command(uint8_t cmd) {

	sNusLongPacket data_array;

	memset(&data_array, 0x00, sizeof(data_array));

	// command
	data_array.nus_data[0] = cmd;
	data_array.data_length = BLE_NUS_STD_DATA_LEN;

	//NRF_LOG_INFO("Queuing cmd %u", cmd);

	while (nrf_queue_is_full(&m_nus_tx_queue)) {
		_process_tx();
	}

	ret_code_t err_code;
	err_code = nrf_queue_push(&m_nus_tx_queue, &data_array);
	APP_ERROR_CHECK(err_code);

}


static void _handle_file_list(void) {

	uint8_t data_array[BLE_NUS_STD_DATA_LEN];
	uint16_t rem_bytes;
	int restart = 1;

	do {

		memset(data_array, 0x00, BLE_NUS_STD_DATA_LEN);

		// command
		data_array[0] = FileListSending;

		sCharArray c_array;
		c_array.str = (char*)&data_array[1];

		rem_bytes = sd_functions__query_fit_list(restart, &c_array, BLE_NUS_STD_DATA_LEN - 1);
		restart = 0;

		NRF_LOG_INFO("Transmitting file list");

		_queue_nus(data_array, BLE_NUS_STD_DATA_LEN);

	} while (rem_bytes > 0); // TODO handle more than 3 files

}

static void _handle_file_upload(uint8_t const *p_data, uint16_t  length) {

	uint8_t data_array[NUS_LONG_PACKETS_SIZE];

	memset(&data_array, 0x00, sizeof(data_array));

	if (!p_data && m_fit_up == FitFileTransferData && !nrf_queue_is_full(&m_nus_tx_queue)) {

		sCharArray c_array;
		c_array.length = 0;
		c_array.str = (char*)&data_array[1];
		uint16_t mtu_length = ble_get_mtu();
		uint16_t long_packet_len = mtu_length < NUS_LONG_PACKETS_SIZE ? mtu_length : NUS_LONG_PACKETS_SIZE;
		long_packet_len -= 1;
		int res = sd_functions__run_query(0, &c_array, long_packet_len);
		if (res || c_array.length < long_packet_len) {
			// we can be here in case of error or if we reached the EOF
			(void)sd_functions__stop_query();
			m_fit_up = FitFileTransferEnd;

			NRF_LOG_INFO("File transfer end (%d) %lu", res, c_array.length);
		} else {

			NRF_LOG_INFO("File transfer continue (%d)", c_array.length);
		}

		// FIT data / end
		// cmd
		// data[1..19]
		data_array[0] = m_fit_up;

		_queue_nus(data_array, c_array.length);

		if (m_fit_up == FitFileTransferEnd) {
			//NRF_LOG_HEXDUMP_INFO(data_array.nus_data, BLE_NUS_STD_DATA_LEN);

			m_fit_up = 0;

			_handle_send_command(SwitchingToLowSpeed);
			_handle_send_command(ConnectedInLowSpeed);
		}

	}

	if (p_data) {

		uint8_t index = 0;

		NRF_LOG_INFO("File transfer start");

		uint32_t f_size = 0;
		char fname[20];
		memset(fname, 0, sizeof(fname));
		snprintf(fname, sizeof(fname), "%08lX.FIT", _decode_uint32_little(p_data+1));
		int res = sd_functions__start_query(eSDTaskQueryFit, fname, &f_size);

		if (res) {
			NRF_LOG_ERROR("FIT file query failed");
			return;
		}

		_handle_send_command(SwitchingToHighSpeed);
		_handle_send_command(ConnectedInHighSpeed);

		// set transfer as started
		m_fit_up  = FitFileTransferStart;

		// Start FIT Xfer
		// cmd
		data_array[index++] = m_fit_up;

		// file id #1
		_encode_uint32_little(_decode_uint32_little(p_data+1), data_array+index);
		index+=4;

		// size[4]
		_encode_uint32_little(f_size, data_array+index);
		index+=4;

		NRF_LOG_HEXDUMP_INFO(data_array, BLE_NUS_STD_DATA_LEN);

		task_delay(500);

		_queue_nus(data_array, BLE_NUS_STD_DATA_LEN);

		m_fit_up = FitFileTransferData;
	}
}

static void _handle_file_delete(uint8_t const *p_data, uint16_t  length) {

	uint8_t data_array[BLE_NUS_STD_DATA_LEN];
	uint8_t index = 0;

	memset(data_array, 0x00, BLE_NUS_STD_DATA_LEN);

	// command
	data_array[index++] = FileDeleteConfirmation;

	// handle delete
	char fname[20];
	memset(fname, 0, sizeof(fname));
	snprintf(fname, sizeof(fname), "%08lX.FIT", _decode_uint32_little(p_data+1));
	int res = sd_functions__start_query(eSDTaskQueryDelete, fname, NULL);

	if (res == 0) {
	// file id #1
	// --> 20 mai 2020
		_encode_uint32_little(_decode_uint32_little(p_data+1), data_array+index);
		index += 4;

		_queue_nus(data_array, BLE_NUS_STD_DATA_LEN);

		NRF_LOG_INFO("Deleting file");
	}
}

static void _handle_phone_status(uint8_t const *p_data, uint16_t  length) {

	int8_t t_zone = (int8_t)p_data[2] / 4;
	uint8_t live_tracking = p_data[3];
	NRF_LOG_INFO("Phone status: GMT+%u hours live-tracking %u", t_zone, live_tracking);
}

static void _send_status_packet(void) {

	uint8_t data_array[BLE_NUS_STD_DATA_LEN];
	uint8_t index = 0;

	memset(data_array, 0x00, BLE_NUS_STD_DATA_LEN);

	// command
	data_array[index++] = RequestPhoneStatus;

	// valueOf8
	//	index += 1;

	// model32
	//	_encode_uint32_little(Y10Super, data_array+index);
	//	index += 4;

	// padding8
	data_array[index++] = 0;

	// major_ver8
	data_array[index++] = 0x6;

	// minor_ver8
	data_array[index++] = 0xA;

	// navigating8
	data_array[index++] = 0xB;

	// gps_mode8
	data_array[index++] = 0x1E;

	// ble_speed8
	//data_array[index++] = 0;

	_queue_nus(data_array, BLE_NUS_STD_DATA_LEN);

	NRF_LOG_INFO("RequestPhoneStatus");
}

static void _handle_segment_list(uint8_t const *p_data, uint16_t  length) {

	if (p_data) {

		m_nb_segs = _decode_uint16_little(&p_data[1]);
	}

	NRF_LOG_INFO("NewSegmentListReady (%u %u)", m_nb_segs);

	inseg_handler_list_reset();

	// command
	_handle_send_command(GPSReadyToReceiveSegmentList);
}

static void _handle_segment_list_item(uint8_t const *p_data, uint16_t  length) {

	uint8_t index = 1;

	// command8
	// 8 bytes: seg ID
	// 4 bytes: i_lat * 1.1930464E7d
	// 4 bytes: i_lon * 1.1930464E7d
	// crc8
	// goal8

	// start LatLon
	index = 9;
	float f_lat = FROM_SEMICIRCLES(_decode_uint32_little(p_data+index));
	index+=4;
	float f_lon = FROM_SEMICIRCLES(_decode_uint32_little(p_data+index));
	index+=4;

	NRF_LOG_HEXDUMP_DEBUG(p_data+1, 19);

	inseg_handler_list_input(&p_data[1], f_lat, f_lon);

	if (p_data[0] == SegmentListItem) {

		NRF_LOG_INFO("SegmentListItem (%ld %ld)", (int32_t)f_lat, (int32_t)f_lon);

	} else {

		NRF_LOG_INFO("SegmentListItemDone (%ld %ld) %u", (int32_t)f_lat, (int32_t)f_lon, m_nb_segs);

		inseg_handler_list_process_start();
	}
}

static void _handle_segment_req_end(void) {

	_handle_send_command(SegmentFileRequestEnd);

	NRF_LOG_INFO("SegmentFileRequestEnd");
}

static void _handle__request_segment(void) {

	uint8_t data_array[BLE_NUS_STD_DATA_LEN];

	memset(&data_array, 0x00, BLE_NUS_STD_DATA_LEN);

	static int prev_res = 1;
	int res = inseg_handler_list_process_tasks(&data_array[1]);

	if (res == 0) {

		// command
		data_array[0] = SegmentFileRequest;

		//NRF_LOG_HEXDUMP_INFO(data_array, BLE_NUS_STD_DATA_LEN);

		task_delay(200);

		_queue_nus(data_array, BLE_NUS_STD_DATA_LEN);

		NRF_LOG_INFO("Sending SegmentFileRequest ...");

	} else if (res > 0 &&
			(prev_res == 0 || prev_res == -2)) {

		_handle_segment_req_end();
	}

	prev_res = res;
}

static void _handle_segment_upload_start(uint8_t const *p_data, uint16_t  length) {

	// cmd8
	// id64
	// size32
	uint32_t seg_size = _decode_uint32_little(&p_data[9]);
	inseg_handler_segment_start(&p_data[1], seg_size);

	NRF_LOG_INFO("SegmentFileUploadStart tot. size=%lu", seg_size);

	// dump ID
	//NRF_LOG_HEXDUMP_INFO(p_data, BLE_NUS_STD_DATA_LEN);
}

//static void _handle_setting_request(uint8_t const *p_data, uint16_t  length) {
//
//    wrap.put(OutgoingCommands.SettingsRequest.byteValue());
//    for (SettingsParser.SettingCategory byteValue : settingCategoryArr) {
//        wrap.put(byteValue.byteValue());
//    }
//
//
//}

static void _handle_segment_upload_data(uint8_t const *p_data, uint16_t length) {

	// cmd8
	// dataX

	if (p_data[0] == SegmentFileUploadEnd) {

		NRF_LOG_INFO("SegmentFileUploadEnd");

	} else {

		NRF_LOG_INFO("SegmentFileUploadData");
	}

	inseg_handler_segment_data(p_data[0] == SegmentFileUploadEnd, p_data+1, length-1);
}

static void _handle_nav(uint8_t const *p_data, uint16_t  length) {


	if (p_data[0] == NavigationNewFile) {

		// cmd8
		// Position32
		// name_size8
		// name[]
		uint16_t index = 1;

		uint32_t dist = _decode_uint32_little(p_data+index);
		index+=4;
		NRF_LOG_INFO("??? : %lu", dist);

		NRF_LOG_INFO("NavigationNewFile");
	} else {

		// cmd8
		// dest_lon32
		// dest_lat32
		// gps_prov8
		// isSavedRoute8
		// z8
		// isSavedRoute8(1) || isTrimmedReroute8(2) || other8(2)
		uint16_t index = 1;

		int32_t i_lon = (int32_t)_decode_uint32_little(p_data+index) / 119;
		index+=4;
		NRF_LOG_INFO("i_lon: %ld", i_lon);

		int32_t i_lat = (int32_t)_decode_uint32_little(p_data+index) / 119;
		index+=4;
		NRF_LOG_INFO("i_lat: %ld", i_lat);

		NRF_LOG_INFO("NavigationNewFileDest");
	}

	NRF_LOG_HEXDUMP_INFO(p_data, length);
}

static void _handle_nav_file(uint8_t const *p_data, uint16_t  length) {

	// cmd8
	// dest_lon32
	// dest_lat32
	// gps_prov8
	// isSavedRoute8(0b1) || isTrimmedReroute8(0b100)
	// poly_length16
	// maneu_length16
	// isLinkRoute8
	// isLinkRoute8 && isTrimmedReroute8  [ char_route[7] + distance32 + linkedRoutePolylineCount16 ]
	// isLinkRoute8 && !isTrimmedReroute8 [ char_route[7] + 0[6] ]
	// polyline[0..X]
	//// lon32
	//// lat32
	//// polylineCount16
	//// maneuverType8
	//// street_name_len8
	//// street_name[1..X]
	//// nav_len16
	//// crc16

	NRF_LOG_INFO("_handle_nav_file %u", p_data[0]);

	switch (p_data[0]) {

	case NavFileUploadDataStart:
	{

	} break;

	case NavFileUploadData:
	{

	} break;

	case NavFileUploadEnd:
	{

	} break;

	default:
		break;

	}

}

static uint8_t _process_tx(void) {

	uint8_t cur_event = 0;
	sNusLongPacket data_array;

	if (!nrf_queue_is_empty(&m_nus_tx_queue)) {

		cur_event |= 2;

		// get the next victim
		ret_code_t err_code = nrf_queue_pop(&m_nus_tx_queue, &data_array);
		APP_ERROR_CHECK(err_code);

//		NRF_LOG_HEXDUMP_INFO(data_array.nus_data, BLE_NUS_STD_DATA_LEN);
//		NRF_LOG_FLUSH();

		do
		{
			if (data_array.data_length == 0) {
				NRF_LOG_ERROR("Empty NUS packet %u", data_array.nus_data[0]);
				break;
			}

			err_code = nus_data_send(data_array.nus_data, data_array.data_length);
			if ((err_code != NRF_ERROR_INVALID_STATE) &&
					(err_code != NRF_ERROR_RESOURCES) &&
					(err_code != NRF_ERROR_NOT_FOUND))
			{
				APP_ERROR_CHECK(err_code);
			}

			if (nrf_qwr_error_get() == NRF_ERROR_RESOURCES) {

				err_code = nrf_qwr_error_get();
				nrf_qwr_error_reset();
			}

			if (err_code == NRF_ERROR_RESOURCES) {

				task_delay(25);
			}

			//artificial delay
			if (err_code == NRF_SUCCESS) {

				NRF_LOG_INFO("Sending packet %u (%lu) size %u",
						data_array.nus_data[0], millis(), data_array.data_length);

				//task_delay(20);
			}

		} while (err_code == NRF_ERROR_RESOURCES);

	}

	return cur_event;
}

void app_handler__signal(void) {

	if (m_rx_task_id != TASK_ID_INVALID) {
		task_delay_cancel(m_rx_task_id);
	}
}

/**@brief Function for handling the data from the UART Service.
 *
 */
void app_handler__nus_data_handler(uint8_t const *p_data, uint16_t length)
{

	sNusPacket data_array;

	memset(&data_array, 0x00, sizeof(sNusPacket));
	memcpy(&data_array, p_data, length);

	NRF_LOG_INFO("RECV %lu (%u) (%u)", millis(), p_data[0], length);

	uint32_t err_code = nrf_queue_push(&m_nus_rx_queue, &data_array);
	APP_ERROR_CHECK(err_code);

	app_handler__signal();

}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
void app_handler__on_connected(void)
{

	_handle_send_command(ConnectedInLowSpeed);
	_send_status_packet();

	app_handler__signal();
}

void app_handler__task(void * p_context) {

	sNusPacket data_array;

	m_rx_task_id = task_id_get();

	task_delay(2000);

	// Start execution.

	while (1) {

		uint8_t cur_event = 0;

		if (!nrf_queue_is_empty(&m_nus_rx_queue)) {

			cur_event = 1;

			// get the next victim
			ret_code_t err_code = nrf_queue_pop(&m_nus_rx_queue, &data_array);
			APP_ERROR_CHECK(err_code);

//			NRF_LOG_HEXDUMP_INFO(data_array.nus_data, BLE_NUS_STD_DATA_LEN);
//			NRF_LOG_FLUSH();

			switch (data_array.nus_data[0]) {

			case RequestFitFileList:
				_handle_file_list();
				break;

			case RequestFitFileDownload:
				_handle_file_upload(data_array.nus_data, BLE_NUS_STD_DATA_LEN);
				break;

			case RequestFitFileDelete:
				_handle_file_delete(data_array.nus_data, BLE_NUS_STD_DATA_LEN);
				break;

			case SettingsRequestV1:
				NRF_LOG_INFO("SettingsRequest");
				break;

			case NewSegmentListReady:
				_handle_segment_list(data_array.nus_data, BLE_NUS_STD_DATA_LEN);
				break;

			case SegmentUpdateCancel:
				inseg_handler_list_reset();
				break;

			case SegmentListItem:
			case SegmentListItemDone:
				_handle_segment_list_item(data_array.nus_data, BLE_NUS_STD_DATA_LEN);
				break;

			case SegmentFileUploadStart:
				_handle_segment_upload_start(data_array.nus_data, BLE_NUS_STD_DATA_LEN);
				break;

			case SegmentFileUploadData:
			case SegmentFileUploadEnd:
				_handle_segment_upload_data(data_array.nus_data, BLE_NUS_STD_DATA_LEN);
				break;

			case NavigationNewFile:
			case NavigationNewFileDest:
				_handle_nav(data_array.nus_data, BLE_NUS_STD_DATA_LEN);
				break;

			case NavFileUploadDataStart:
			case NavFileUploadData:
			case NavFileUploadEnd:
				_handle_nav_file(data_array.nus_data, BLE_NUS_STD_DATA_LEN);
				break;

			case PhoneStatus:
				_handle_phone_status(data_array.nus_data, BLE_NUS_STD_DATA_LEN);
				break;

			default:
			{
				NRF_LOG_INFO("Received data from BLE NUS => cmd=%u", data_array.nus_data[0]);
				NRF_LOG_HEXDUMP_INFO(data_array.nus_data, BLE_NUS_STD_DATA_LEN);
			}
			break;
			}

		}

		cur_event |= _process_tx();

		// possibly queue new packets
		_handle_file_upload(NULL, 0);

		_handle__request_segment();

		if (cur_event == 0) {
			task_delay(100);
		}

	}
}



