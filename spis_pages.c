/*
 * spis_pages.c
 *
 *  Created on: 6 déc. 2017
 *      Author: Vincent
 */

#include "helper.h"
#include "spis.h"
#include "spis_pages.h"


#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

/**
 *
 * @param info
 */
void spis_encode_lns(sLnsInfo* info) {
	m_tx_buf[TX_BUFF_FLAGS_POS] |= 1 << TX_BUFF_FLAGS_LNS_BIT;

	encode_uint32 (m_tx_buf + TX_BUFF_LNS_START +  0, (uint32_t) info->lat);
	encode_uint32 (m_tx_buf + TX_BUFF_LNS_START +  4, (uint32_t) info->lon);
	encode_uint32 (m_tx_buf + TX_BUFF_LNS_START +  8, (uint32_t) info->ele);
	encode_uint16 (m_tx_buf + TX_BUFF_LNS_START + 12, (uint16_t) info->speed);
	encode_uint32 (m_tx_buf + TX_BUFF_LNS_START + 14, info->secj);
	encode_uint32 (m_tx_buf + TX_BUFF_LNS_START + 18, info->date);
}

/**
 *
 * @param info
 */
void spis_encode_hrm(sHrmInfo* info) {
	m_tx_buf[TX_BUFF_FLAGS_POS] |= 1 << TX_BUFF_FLAGS_HRM_BIT;

	m_tx_buf[TX_BUFF_HRM_START] = info->bpm;
	encode_uint16 (m_tx_buf + TX_BUFF_HRM_START + 1, info->rr);
}

/**
 *
 * @param info
 */
void spis_encode_bsc(sBscInfo* info) {
	m_tx_buf[TX_BUFF_FLAGS_POS] |= 1 << TX_BUFF_FLAGS_BSC_BIT;

	encode_uint32 (m_tx_buf + TX_BUFF_BSC_START + 0, info->cadence);
	encode_uint32 (m_tx_buf + TX_BUFF_BSC_START + 4, info->speed);
}

/**
 *
 * @param info
 */
void spis_encode_fec(sFecInfo* info) {
	m_tx_buf[TX_BUFF_FLAGS_POS] |= 1 << TX_BUFF_FLAGS_FEC_BIT;

	encode_uint16 (m_tx_buf + TX_BUFF_FEC_START + 0, info->el_time);
	encode_uint16 (m_tx_buf + TX_BUFF_FEC_START + 2, info->speed);
	encode_uint16 (m_tx_buf + TX_BUFF_FEC_START + 4, info->power);
}

/**
 *
 * @param rx_buf
 * @param output
 */
static void spis_decode_page0(uint8_t *rx_buf, sSpisRxInfo *output) {
	output->page_id = eSpiRxPage0;

	output->pages.page0.batt_info.soc = rx_buf[RX_BUFF_BAT_START];
	output->pages.page0.batt_info.mv  = decode_uint16 (rx_buf + RX_BUFF_BAT_START + 1U);

	output->pages.page0.glasses_info.led    = rx_buf[RX_BUFF_GLA_START];
	output->pages.page0.glasses_info.av_ent = rx_buf[RX_BUFF_GLA_START + 1U];
	output->pages.page0.glasses_info.av_dec = rx_buf[RX_BUFF_GLA_START + 2U];

	output->pages.page0.neo_info.event_type = rx_buf[RX_BUFF_NEO_START];
	output->pages.page0.neo_info.on_time    = rx_buf[RX_BUFF_NEO_START + 1U];
	output->pages.page0.neo_info.rgb[0]     = rx_buf[RX_BUFF_NEO_START + 2U];
	output->pages.page0.neo_info.rgb[1]     = rx_buf[RX_BUFF_NEO_START + 3U];
	output->pages.page0.neo_info.rgb[2]     = rx_buf[RX_BUFF_NEO_START + 4U];

	output->pages.page0.fec_info.type       = rx_buf[RX_BUFF_FEC_START];
	if (output->pages.page0.fec_info.type == eFecControlTargetPower) {
		output->pages.page0.fec_info.data.power_control.target_power_w = decode_uint16 (rx_buf + RX_BUFF_FEC_START + 1U);
	} else if (eFecControlSlope) {
		output->pages.page0.fec_info.data.slope_control.slope_ppc = (float)decode_uint32 (rx_buf + RX_BUFF_FEC_START + 1U);
		output->pages.page0.fec_info.data.slope_control.rolling_resistance = (float)decode_uint32 (rx_buf + RX_BUFF_FEC_START + 5U);
	}

	output->pages.page0.back_info.freq      = rx_buf[RX_BUFF_BACK_START];
	output->pages.page0.back_info.state     = rx_buf[RX_BUFF_BACK_START + 1U];

	output->pages.page0.power_info.state    = rx_buf[RX_BUFF_SWITCH_START];
}

/**
 *
 * @param rx_buf
 * @param output
 */
static void spis_decode_page1(uint8_t *rx_buf, sSpisRxInfo *output) {
	output->page_id = eSpiRxPage1;
}

/** TODO
 *
 * @param rx_buf
 * @param output
 */
void spis_decode_rx_page(uint8_t *rx_buf, sSpisRxInfo *output) {

	switch (rx_buf[RX_BUFF_PAGE_POS]) {
	case eSpiRxPage0:
		spis_decode_page0(rx_buf, output);
		break;

	case eSpiRxPage1:
		spis_decode_page1(rx_buf, output);
		break;

	default:
	{
		NRF_LOG_WARNING("Unknown SPIS page %u", rx_buf[RX_BUFF_PAGE_POS]);
		for (uint8_t i = 0; i < 64; i++) {
			NRF_LOG_RAW_INFO("%02X ", rx_buf[i]);
		}
		NRF_LOG_RAW_INFO("\r\n");
		output->page_id = eSpiRxPageInv;
	}
		break;
	}


}

