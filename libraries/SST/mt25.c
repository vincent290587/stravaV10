
#include "boards.h"

#if defined (PROTO_V10)

#include <stdbool.h>
#include "nrfx_qspi.h"
#include "nor_defines.h"
#include "segger_wrapper.h"
#include "nrf_serial_flash_params.h"



static const nrf_serial_flash_params_t m_sflash_params[] = {
    {    /*MT25*/
        .read_id = { 0x20, 0xBA, 0x18 },
        .capabilities = 0x00,
        .size = 128 * 1024 * 1024 / 8,
        .erase_size = 4 * 1024,
        .program_size = 256,
    }
};

nrf_serial_flash_params_t const * nrf_serial_flash_params_get(const uint8_t * p_read_id)
{
    size_t i;

    for (i = 0; i < ARRAY_SIZE(m_sflash_params); ++i)
    {
        if (memcmp(m_sflash_params[i].read_id, p_read_id, sizeof(m_sflash_params[i].read_id)) == 0)
        {
            return &m_sflash_params[i];
        }
    }

    LOG_ERROR("Read params: 0x%02X 0x%02X 0x%02X",
    		p_read_id[0], p_read_id[1], p_read_id[2]);

    return NULL;
}


bool configure_memory()
{
	uint32_t err_code;
	uint8_t recv[3];
    uint8_t conf_reg[2] = {0};
    uint8_t stat_reg = 0;
	nrf_qspi_cinstr_conf_t cinstr_cfg = {
			.opcode    = QSPI_STD_CMD_RSTEN,
			.length    = NRF_QSPI_CINSTR_LEN_1B,
			.io2_level = true,
			.io3_level = true,
			.wipwait   = true,
			.wren      = false
	};

    // Exit XIP & QSPI
	for (int i=0; i < 6; i++) {
		cinstr_cfg.opcode = 0xFF;
		cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_1B;
		err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
		APP_ERROR_CHECK(err_code);
	}

	// Reset QIO
    cinstr_cfg.opcode = QSPI_STD_CMD_RSTEN;
    stat_reg = 0xF5;
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_2B;
	err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, &stat_reg, NULL);
	APP_ERROR_CHECK(err_code);

	// Send reset enable
    cinstr_cfg.opcode = QSPI_STD_CMD_RSTEN;
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_1B;
	err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
	APP_ERROR_CHECK(err_code);

	// Send reset command
	cinstr_cfg.opcode = QSPI_STD_CMD_RST;
	err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
	APP_ERROR_CHECK(err_code);

	// Get device ID
	cinstr_cfg.opcode = 0xAF;
	cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_4B;
	err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, recv);
	APP_ERROR_CHECK(err_code);
	LOG_INFO("JEDEC ID: 0x%02X 0x%02X 0x%02X", recv[0], recv[1], recv[2]);

	// write status register
    cinstr_cfg.opcode = 0x01;
    stat_reg = 0;
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_2B;
    err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, &stat_reg, NULL);
    APP_ERROR_CHECK(err_code);

	// write conf register
    cinstr_cfg.opcode = 0xB1;
    conf_reg[1] = 0xFF;
    conf_reg[0] = 0xFF;
    conf_reg[0] &= ~(1 << 2);
    conf_reg[0] &= ~(1 << 3);
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_3B;
    err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, &conf_reg, NULL);
    APP_ERROR_CHECK(err_code);

    LOG_INFO("Write conf register: 0x%02X%02X", conf_reg[1], conf_reg[0]);

	// write enhanced volatile conf. register
    cinstr_cfg.opcode = 0x61;
    stat_reg = 0x3F;
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_2B;
    err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, &stat_reg, NULL);
    APP_ERROR_CHECK(err_code);

    // Switch to 4-io-qspi mode
//    cinstr_cfg.opcode = 0x35;
//    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_1B;
//    err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
//    APP_ERROR_CHECK(err_code);

	return recv[0] == 0x20;
}

#endif

