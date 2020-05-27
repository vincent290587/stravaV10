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
#include "nrf_queue.h"

#include "task_manager.h"

#include "lezyne_protocol.h"
#include "app_packets_handler.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


typedef struct {
	uint32_t cur_size;
	uint32_t total_size;
	uint8_t  seg_id[8];
	uint8_t  buffer[10000];
} sUploadSegment;


typedef struct {
	uint8_t nus_data[BLE_NUS_MAX_DATA_LEN];
	uint16_t data_length;
} sNusPacket;

task_id_t m_rx_task_id = TASK_ID_INVALID;

NRF_QUEUE_DEF(sNusPacket, m_nus_tx_queue, 4, NRF_QUEUE_MODE_NO_OVERFLOW);
NRF_QUEUE_DEF(sNusPacket, m_nus_rx_queue, 20, NRF_QUEUE_MODE_NO_OVERFLOW);


static sUploadSegment m_cur_u_seg;

static uint16_t m_nb_segs = 0;
static int8_t m_fit_up = 0;
static uint16_t m_fit_ptr = 0;

// 631065600uL is GMT: Sunday 31 December 1989 00:00:00

// left part is unix timestamp in SECONDS
// 958 647 620uL + 631 065 600uL = 1 589 713 220 ===> GMT: Sunday 17 May 2020 11:00:20 :  GOOD !
static uint32_t m_fake_name = 958647621uL + 1; // FIT timestamp

// 959358152 <===> 392EA4C7 from Lezyne device

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

	sNusPacket data_array;

	memcpy(&data_array, p_data, length);
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

	sNusPacket data_array;

	memset(&data_array, 0x00, sizeof(data_array));

	// command
	data_array.nus_data[0] = cmd;
	data_array.data_length = 20;

	//NRF_LOG_INFO("Queuing cmd %u", cmd);

	while (nrf_queue_is_full(&m_nus_tx_queue)) {
		_process_tx();
	}

	ret_code_t err_code;
	err_code = nrf_queue_push(&m_nus_tx_queue, &data_array);
	APP_ERROR_CHECK(err_code);

}


static void _handle_file_list(void) {

	uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
	uint8_t index = 0;

	memset(data_array, 0x00, BLE_NUS_MAX_DATA_LEN);

	// command
	data_array[index++] = FileListSending;

	// TODO nb files
	data_array[index++] = 1;

	// file id #1
	// --> 20 mai 2020
	_encode_uint32_little(m_fake_name, data_array+index);

	NRF_LOG_INFO("Transmitting file list");

	_queue_nus(data_array, BLE_NUS_MAX_DATA_LEN);

}

static void _handle_file_upload(uint8_t const *p_data, uint16_t  length) {

	sNusPacket data_array;

	memset(&data_array, 0x00, sizeof(data_array));

	if (!p_data && m_fit_up == FitFileTransferData && !nrf_queue_is_full(&m_nus_tx_queue)) {

		int16_t incr = 19;

		// FIT data / end
		// cmd
		// data[1..19]
		if (m_fit_ptr + incr >= sizeof(m_fit_file)) {

			incr = sizeof(m_fit_file) - m_fit_ptr;
			m_fit_up = FitFileTransferEnd;

			NRF_LOG_INFO("File transfer end %u (%d)", m_fit_ptr, incr);

		} else {

			NRF_LOG_INFO("File transfer continue %u (%d)", m_fit_ptr, incr);
		}

		data_array.nus_data[0] = m_fit_up;
		for (int16_t i=0; i< incr; i++) {
			data_array.nus_data[1+i] = m_fit_file[m_fit_ptr++];
		}

		_queue_nus(data_array.nus_data, 20);

		if (m_fit_up == FitFileTransferEnd) {
			//NRF_LOG_HEXDUMP_INFO(data_array.nus_data, 20);

			m_fit_up = 0;

			NRF_LOG_INFO("%u bytes xferred", m_fit_ptr);

			_handle_send_command(SwitchingToLowSpeed);
			_handle_send_command(ConnectedInLowSpeed);
		}

	}

	if (p_data) {

		uint8_t index = 0;

		NRF_LOG_INFO("File transfer start");

		_handle_send_command(SwitchingToHighSpeed);
		_handle_send_command(ConnectedInHighSpeed);

		// set transfer as started
		m_fit_up  = FitFileTransferStart;
		m_fit_ptr = 0;

		// Start FIT Xfer
		// cmd
		data_array.nus_data[index++] = m_fit_up;

		// file id #1
		// --> 20 mai 2020
		_encode_uint32_little(m_fake_name, data_array.nus_data+index);
		index+=4;

		// size[4]
		// we want a multiple of 19 here
		uint32_t length = sizeof(m_fit_file);
		_encode_uint32_little(length, data_array.nus_data+index);
		index+=4;

		NRF_LOG_HEXDUMP_INFO(data_array.nus_data, sizeof(data_array));

		task_delay(500);

		_queue_nus(data_array.nus_data, 20);

		m_fit_up = FitFileTransferData;
	}
}

static void _handle_file_delete(uint8_t const *p_data, uint16_t  length) {

	uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
	uint8_t index = 0;

	memset(data_array, 0x00, BLE_NUS_MAX_DATA_LEN);

	// command
	data_array[index++] = FileDeleteConfirmation;

	// file id #1
	// --> 20 mai 2020
	_encode_uint32_little(m_fake_name, data_array+index);
	index += 4;

	_queue_nus(data_array, BLE_NUS_MAX_DATA_LEN);

	NRF_LOG_INFO("Deleting file");
}

static void _handle_phone_status(uint8_t const *p_data, uint16_t  length) {

	int8_t t_zone = (int8_t)p_data[2] / 4;
	uint8_t live_tracking = p_data[3];
	NRF_LOG_INFO("Phone status: GMT+%u hours live-tracking %u", t_zone, live_tracking);
}

static void _send_status_packet(void) {

	uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
	uint8_t index = 0;

	memset(data_array, 0x00, BLE_NUS_MAX_DATA_LEN);

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

	_queue_nus(data_array, BLE_NUS_MAX_DATA_LEN);

	NRF_LOG_INFO("RequestPhoneStatus");
}

static void _handle_segment_list(uint8_t const *p_data, uint16_t  length) {

	if (p_data) {

		m_nb_segs = _decode_uint16_little(&p_data[1]);
	}

	NRF_LOG_INFO("NewSegmentListReady (%u %u)", m_nb_segs);

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

	NRF_LOG_HEXDUMP_DEBUG(p_data+9, 8);

	if (p_data[0] == SegmentListItem) {

		// start LatLon
		index = 9;
		int32_t i_lat = (int32_t)_decode_uint32_little(p_data+index) / 119;
		index+=4;
		int32_t i_lon = (int32_t)_decode_uint32_little(p_data+index) / 119;
		index+=4;

		NRF_LOG_INFO("SegmentListItem (%ld %ld)", i_lat, i_lon);

	} else {

		// stop LatLon
		index = 9;
		int32_t i_lat = (int32_t)_decode_uint32_little(p_data+index) / 119;
		index+=4;
		int32_t i_lon = (int32_t)_decode_uint32_little(p_data+index) / 119;
		index+=4;

		NRF_LOG_INFO("SegmentListItemDone (%ld %ld) %u", i_lat, i_lon, m_nb_segs);

		// TODO store all segs names
		//uint64_t *seg_id = (uint64_t *)(m_cur_u_seg.seg_id);

		if (m_nb_segs > 0) {

			uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
			index = 0;

			memset(&data_array, 0x00, BLE_NUS_MAX_DATA_LEN);

			// command
			data_array[0] = SegmentFileRequest;

			memcpy(m_cur_u_seg.seg_id, p_data+1, sizeof(m_cur_u_seg.seg_id));
			memcpy(&data_array[1], m_cur_u_seg.seg_id, sizeof(m_cur_u_seg.seg_id));

			//NRF_LOG_HEXDUMP_INFO(data_array, 20);

			task_delay(300);

			_queue_nus(data_array, 20);

			NRF_LOG_INFO("Sending SegmentFileRequest ...");
		}
	}

	NRF_LOG_HEXDUMP_DEBUG(p_data+1, 8);
}

static void _handle_segment_parse(void) {

	uint8_t index = 0;

	// start_lat32
	// start_lon32
	// end_lat32
	// end_lon32
	// grade16
	// 00 00 00
	// name_length8
	// name[1..19]
	// poly_length16
	// poly[]
	// pr_time16
	// kom_time16
	// isHazardous16
	// tot_compressed_nb_words16
	// cStrDist_length16
	// compressedStreamDistance[]
	// cStrEffort_length16
	// compressedStreamEffort[]

	int32_t i_lat = (int32_t)_decode_uint32_little(m_cur_u_seg.buffer+index) / 119;
	index+=4;
	int32_t i_lon = (int32_t)_decode_uint32_little(m_cur_u_seg.buffer+index) / 119;
	index+=4;

	index+=8; // not interested..?

	int16_t grade = (int16_t)_decode_uint16_little(m_cur_u_seg.buffer+index);
	index+=2;

	index+=3; // padding 0

	// dump name
	uint16_t name_length = m_cur_u_seg.buffer[index];
	index+=1;

	NRF_LOG_HEXDUMP_INFO(m_cur_u_seg.buffer+index, name_length); // name
	index+=name_length;

	uint16_t poly_length = _decode_uint16_little(m_cur_u_seg.buffer+index);
	index+=2;

	NRF_LOG_HEXDUMP_INFO(m_cur_u_seg.buffer+index, poly_length);
	index+=poly_length;

	NRF_LOG_INFO("Polyline length: %u", poly_length);

	// pr time
	uint16_t pr_time =  _decode_uint16_little(m_cur_u_seg.buffer+index);
	index+=2;

	// kom time
	uint16_t kom_time =  _decode_uint16_little(m_cur_u_seg.buffer+index);
	index+=2;

	NRF_LOG_INFO("PR/KOM times: %u %u", pr_time, kom_time);

	index+=2; // hazardous

	uint16_t words_comp =  _decode_uint16_little(m_cur_u_seg.buffer+index);
	index+=2;

	uint16_t comp_dist_length = _decode_uint16_little(m_cur_u_seg.buffer+index);
	index+=2;

	NRF_LOG_INFO("Dist stream length: %u", comp_dist_length);

	index+=comp_dist_length;

	uint16_t comp_stream_length = _decode_uint16_little(m_cur_u_seg.buffer+index);
	index+=2;

	NRF_LOG_INFO("Effort stream length: %u", comp_dist_length);

	index+=comp_stream_length;
}

static void _handle_segment_upload_start(uint8_t const *p_data, uint16_t  length) {

	uint8_t index = 1;

	// cmd8
	// id64
	// size32

	// copy segment ID
	memcpy(m_cur_u_seg.seg_id, p_data + index, 8);
	index+=8;

	m_cur_u_seg.cur_size = 0;
	m_cur_u_seg.total_size = _decode_uint32_little(p_data+index);

	NRF_LOG_INFO("SegmentFileUploadStart tot. size=%lu", m_cur_u_seg.total_size);

	// dump ID
	//NRF_LOG_HEXDUMP_INFO(p_data, 20);
}

static void _handle_setting_request(uint8_t const *p_data, uint16_t  length) {

	//    wrap.put(OutgoingCommands.SettingsRequest.byteValue());
	//    for (SettingsParser.SettingCategory byteValue : settingCategoryArr) {
	//        wrap.put(byteValue.byteValue());
	//    }


}

static void _handle_segment_req_end(void) {


	memset(&m_cur_u_seg, 0, sizeof(m_cur_u_seg));

	_handle_send_command(SegmentFileRequestEnd);

	NRF_LOG_INFO("SegmentFileRequestEnd");
}

static void _handle_segment_upload_data(uint8_t const *p_data, uint16_t  length) {

	// cmd8
	// dataX

	if (p_data[0] == SegmentFileUploadEnd) {

		size_t incr = m_cur_u_seg.total_size - m_cur_u_seg.cur_size;

		// copy segment data
		memcpy(m_cur_u_seg.buffer+m_cur_u_seg.cur_size, p_data + 1, incr);

		m_cur_u_seg.cur_size += incr;

		NRF_LOG_INFO("SegmentFileUploadEnd %lu / %lu", m_cur_u_seg.cur_size, m_cur_u_seg.total_size);

		_handle_segment_parse();

		// ACK that the file was received
		_handle_segment_req_end();
	} else {

		// copy segment data
		memcpy(m_cur_u_seg.buffer+m_cur_u_seg.cur_size, p_data + 1, 19);

		m_cur_u_seg.cur_size += 19;

		NRF_LOG_INFO("SegmentFileUploadData %lu / %lu", m_cur_u_seg.cur_size, m_cur_u_seg.total_size);
	}
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
	sNusPacket data_array;

	if (!nrf_queue_is_empty(&m_nus_tx_queue)) {

		cur_event |= 2;

		// get the next victim
		ret_code_t err_code = nrf_queue_pop(&m_nus_tx_queue, &data_array);
		APP_ERROR_CHECK(err_code);

//		NRF_LOG_HEXDUMP_INFO(data_array.nus_data, 20);
//		NRF_LOG_FLUSH();

		do
		{

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

				NRF_LOG_INFO("Sending packet %u (%lu)",
						data_array.nus_data[0], millis());

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

	NRF_LOG_INFO("RECV %lu (%u)", millis(), p_data[0]);

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

	NRF_LOG_INFO("handler starting iD %u", m_rx_task_id);

	// Start execution.

	while (1) {

		uint8_t cur_event = 0;

		if (!nrf_queue_is_empty(&m_nus_rx_queue)) {

			cur_event = 1;

			// get the next victim
			ret_code_t err_code = nrf_queue_pop(&m_nus_rx_queue, &data_array);
			APP_ERROR_CHECK(err_code);

//			NRF_LOG_HEXDUMP_INFO(data_array.nus_data, 20);
//			NRF_LOG_FLUSH();

			switch (data_array.nus_data[0]) {

			case RequestFitFileList:
				_handle_file_list();
				break;

			case RequestFitFileDownload:
				_handle_file_upload(data_array.nus_data, BLE_NUS_MAX_DATA_LEN);
				break;

			case RequestFitFileDelete:
				_handle_file_delete(data_array.nus_data, BLE_NUS_MAX_DATA_LEN);
				break;

			case SettingsRequestV1:
				NRF_LOG_INFO("SettingsRequest");
				break;

			case NewSegmentListReady:
				_handle_segment_list(data_array.nus_data, BLE_NUS_MAX_DATA_LEN);
				break;

			case SegmentListItem:
			case SegmentListItemDone:
				_handle_segment_list_item(data_array.nus_data, BLE_NUS_MAX_DATA_LEN);
				break;

			case SegmentFileUploadStart:
				_handle_segment_upload_start(data_array.nus_data, BLE_NUS_MAX_DATA_LEN);
				break;

			case SegmentFileUploadData:
			case SegmentFileUploadEnd:
				_handle_segment_upload_data(data_array.nus_data, BLE_NUS_MAX_DATA_LEN);
				break;

			case NavigationNewFile:
			case NavigationNewFileDest:
				_handle_nav(data_array.nus_data, BLE_NUS_MAX_DATA_LEN);
				break;

			case NavFileUploadDataStart:
			case NavFileUploadData:
			case NavFileUploadEnd:
				_handle_nav_file(data_array.nus_data, BLE_NUS_MAX_DATA_LEN);
				break;

			case PhoneStatus:
				_handle_phone_status(data_array.nus_data, BLE_NUS_MAX_DATA_LEN);
				break;

			default:
			{
				NRF_LOG_INFO("Received data from BLE NUS => cmd=%u", data_array.nus_data[0]);
				NRF_LOG_HEXDUMP_INFO(data_array.nus_data, BLE_NUS_MAX_DATA_LEN);
			}
			break;
			}

		}

		cur_event |= _process_tx();

		// possibly queue new packets
		_handle_file_upload(NULL, 0);

		if (cur_event == 0) {
			task_delay(20);
		}

	}
}



