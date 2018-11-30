
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
