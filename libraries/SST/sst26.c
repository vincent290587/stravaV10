
#include "boards.h"

#if !defined (PROTO_V10)

#include <stdbool.h>
#include "nrfx_qspi.h"
#include "nor_defines.h"
#include "segger_wrapper.h"
#include "nrf_serial_flash_params.h"


static const nrf_serial_flash_params_t m_sflash_params[] = {
    {    /*SST26VF*/
        .read_id = { 0xBF, 0x26, 0x42 },
        .capabilities = 0x00,
        .size = 32 * 1024 * 1024 / 8,
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
	nrf_qspi_cinstr_conf_t cinstr_cfg = {
			.opcode    = QSPI_STD_CMD_RSTEN,
			.length    = NRF_QSPI_CINSTR_LEN_1B,
			.io2_level = true,
			.io3_level = true,
			.wipwait   = true,
			.wren      = true
	};

	// Send reset enable
	err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
	APP_ERROR_CHECK(err_code);

	// Send reset command
	cinstr_cfg.opcode = QSPI_STD_CMD_RST;
	err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
	APP_ERROR_CHECK(err_code);

	delay_ms(3);

	// Get device ID
	cinstr_cfg.opcode = 0x9F;
	cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_4B;
	uint8_t recv[3] = {0};
	err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, recv);
	APP_ERROR_CHECK(err_code);
	LOG_INFO("JEDEC ID: 0x%02X 0x%02X 0x%02X", recv[0], recv[1], recv[2]);

	// write status register
    cinstr_cfg.opcode = 0x01;
    uint8_t temporary[2] = {0};
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_3B;
    err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, &temporary, NULL);
    APP_ERROR_CHECK(err_code);

	// Global unlock SST26VF
	cinstr_cfg.opcode = 0x98;
	cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_1B;
	err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
	APP_ERROR_CHECK(err_code);

    // Switch to 4-io-qspi mode
//    cinstr_cfg.opcode = 0x38;
//    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_1B;
//    err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
//    APP_ERROR_CHECK(err_code);

	return recv[0] == 0xBF;
}

#endif
