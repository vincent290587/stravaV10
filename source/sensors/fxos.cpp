/*
 * FXOS.cpp
 *
 *  Created on: 12 nov. 2017
 *      Author: Vincent
 */

#include "i2c.h"
#include "nrf_twi_mngr.h"
#include "fsl_fxos.h"
#include "gpio.h"
#include "nrfx_gpiote.h"
#include "boards.h"
#include "RingBuffer.h"
#include "UserSettings.h"
#include "segger_wrapper.h"
#include "fxos.h"
#include "Model.h"
#include <math.h>
#include <millis.h>

#define EN_INTERRUPTS
#define LPSLEEP_HIRES

#define kStatus_Fail    1
#define kStatus_Success 0

// Buffer for data read from sensors.
static fxos_handle_t m_fxos_handle;

static tHistoValue _pi_buffer[PITCH_BUFFER_SIZE];
RingBuffer<tHistoValue> m_pitch_buffer(PITCH_BUFFER_SIZE, _pi_buffer);


static void _convert_samples(fxos_data_t *fxos_data, int16_t *Ax, int16_t *Ay, int16_t *Az, int16_t *Mx, int16_t *My, int16_t *Mz);

ret_code_t FXOS_ReadReg(fxos_handle_t *handle, uint8_t reg, uint8_t *val, uint8_t bytesNumber)
{
	ret_code_t status = NRF_SUCCESS;

#ifdef _DEBUG_TWI
	if (!i2c_read_reg_n(FXOS_7BIT_ADDRESS, reg, val, bytesNumber)) {
		LOG_ERROR("FXOS Error 1");
		status = 1;
	}
#endif

	return status;
}

ret_code_t FXOS_WriteReg(fxos_handle_t *handle, uint8_t reg, uint8_t val)
{
	ret_code_t status = NRF_SUCCESS;

#ifdef _DEBUG_TWI
	if (!i2c_write_reg_8(FXOS_7BIT_ADDRESS, reg, val)) {
		status = 1;
	}
#else

#endif

	return status;
}

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define MAX_ACCEL_AVG_COUNT 75U

/* multiplicative conversion constants */
#define DegToRad 0.017453292f
#define RadToDeg 57.295779f


/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint8_t g_sensorRange = FULL_SCALE_4G;
static uint8_t g_dataScale   = 4U;

float g_Mx_Raw = 0;
float g_My_Raw = 0;
float g_Mz_Raw = 0;

static float g_Yaw = 0;
static float g_Yaw_LP = 0;
static float g_Pitch = 0;
static float g_Roll = 0;

static int16_t g_Ax_buff[MAX_ACCEL_AVG_COUNT] = {0};
static int16_t g_Ay_buff[MAX_ACCEL_AVG_COUNT] = {0};
static int16_t g_Az_buff[MAX_ACCEL_AVG_COUNT] = {0};

static int16_t g_Mx_buff[MAX_ACCEL_AVG_COUNT] = {0};
static int16_t g_My_buff[MAX_ACCEL_AVG_COUNT] = {0};
static int16_t g_Mz_buff[MAX_ACCEL_AVG_COUNT] = {0};

static int16_t g_Mx_Offset = 0;
static int16_t g_My_Offset = 0;
static int16_t g_Mz_Offset = 0;


static int16_t Mx_max = 0;
static int16_t My_max = 0;
static int16_t Mz_max = 0;
static int16_t Mx_min = 0;
static int16_t My_min = 0;
static int16_t Mz_min = 0;

static bool g_Calibration_Done = false;


/***************************************************************************
 C FUNCTIONS
 ***************************************************************************/

static inline void _process_fxos_measures(fxos_handle_t *p_fxosHandle) {

	int16_t _Ax_Raw = 0;
	int16_t _Ay_Raw = 0;
	int16_t _Az_Raw = 0;

	int16_t _Mx_Raw = 0;
	int16_t _My_Raw = 0;
	int16_t _Mz_Raw = 0;

	/* Read sensor data */
	_convert_samples(&p_fxosHandle->data, &_Ax_Raw, &_Ay_Raw, &_Az_Raw, &_Mx_Raw, &_My_Raw, &_Mz_Raw);

	/* Average accel value */
	for (uint16_t i = 1; i < MAX_ACCEL_AVG_COUNT; i++)
	{
		g_Ax_buff[i] = g_Ax_buff[i - 1];
		g_Ay_buff[i] = g_Ay_buff[i - 1];
		g_Az_buff[i] = g_Az_buff[i - 1];

		g_Mx_buff[i] = g_Mx_buff[i - 1];
		g_My_buff[i] = g_My_buff[i - 1];
		g_Mz_buff[i] = g_Mz_buff[i - 1];
	}

	g_Ax_buff[0] = _Ax_Raw;
	g_Ay_buff[0] = _Ay_Raw;
	g_Az_buff[0] = _Az_Raw;

	g_Mx_buff[0] = _Mx_Raw;
	g_My_buff[0] = _My_Raw;
	g_Mz_buff[0] = _Mz_Raw;

}

static void _int_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	W_SYSVIEW_RecordEnterISR();

	LOG_DEBUG("FXOS int %u", millis());

	fxos_readChip();

	W_SYSVIEW_RecordEnterISR();
}

static void _fxos_read_cb(ret_code_t result, void * p_user_data) {

	APP_ERROR_CHECK(result);
	if (result) return;

	// we fill the internal buffers, using oversampling
	_process_fxos_measures(&m_fxos_handle);

}

void fxos_readChip(void) {

    // [these structures have to be "static" - they cannot be placed on stack
    //  since the transaction is scheduled and these structures most likely
    //  will be referred after this function returns]

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND fxos_regs[] = { STATUS_00_REG };

    static nrf_twi_mngr_transfer_t const transfers[] =
    {
    		I2C_READ_REG(FXOS_7BIT_ADDRESS, fxos_regs, &m_fxos_handle, sizeof(m_fxos_handle)),
    };
    static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction =
    {
        .callback            = _fxos_read_cb,
        .p_user_data         = NULL,
        .p_transfers         = transfers,
        .number_of_transfers = sizeof(transfers) / sizeof(transfers[0])
    };

    i2c_schedule(&transaction);

}

/*!
 * @brief Read all data from sensor function
 *
 * @param Ax The pointer store x axis acceleration value
 * @param Ay The pointer store y axis acceleration value
 * @param Az The pointer store z axis acceleration value
 * @param Mx The pointer store x axis magnetic value
 * @param My The pointer store y axis magnetic value
 * @param Mz The pointer store z axis magnetic value
 * @note Must calculate g_dataScale before use this function.
 */
static void _convert_samples(fxos_data_t *fxos_data, int16_t *Ax, int16_t *Ay, int16_t *Az, int16_t *Mx, int16_t *My, int16_t *Mz)
{
#ifdef _DEBUG_TWI
	if (FXOS_ReadReg(nullptr, OUT_X_MSB_REG, &fxos_data->accelXMSB, 6)) {
		LOG_ERROR("Error reading OUT_X_MSB_REG");
		return;
	}
	if (FXOS_ReadReg(nullptr, M_OUT_X_MSB_REG, &fxos_data->magXMSB, 6)) {
		LOG_ERROR("Error reading M_OUT_X_MSB_REG");
		return;
	}
	uint8_t temp = 0;
	if (FXOS_ReadReg(nullptr, TEMP_REG, &temp, 1)) {
		LOG_ERROR("Error reading TEMP_REG");
		return;
	} else {
		LOG_INFO("FXOS temp: %u", temp);
	}

#else
	ASSERT(fxos_data);
#endif

	/* Get the accel data from the sensor data structure in 14 bit left format data*/
	*Ax = (int16_t)(((fxos_data->accelXMSB << 8) | fxos_data->accelXLSB)) >> 2u;
	*Ay = (int16_t)(((fxos_data->accelYMSB << 8) | fxos_data->accelYLSB)) >> 2u;
	*Az = (int16_t)(((fxos_data->accelZMSB << 8) | fxos_data->accelZLSB)) >> 2u;

	*Mx = (int16_t)((fxos_data->magXMSB << 8) | (uint16_t)fxos_data->magXLSB);
	*My = (int16_t)((fxos_data->magYMSB << 8) | (uint16_t)fxos_data->magYLSB);
	*Mz = (int16_t)((fxos_data->magZMSB << 8) | (uint16_t)fxos_data->magZLSB);

}

static int Magnetometer_Calibrate_Task(void)
{

	//if (!g_Calibration_Done)
	{
		if (g_Mx_Raw > Mx_max)
		{
			Mx_max = g_Mx_Raw;
		}
		if (g_My_Raw > My_max)
		{
			My_max = g_My_Raw;
		}
		if (g_Mz_Raw > Mz_max)
		{
			Mz_max = g_Mz_Raw;
		}
		if (g_Mx_Raw < Mx_min)
		{
			Mx_min = g_Mx_Raw;
		}
		if (g_My_Raw < My_min)
		{
			My_min = g_My_Raw;
		}
		if (g_Mz_Raw < Mz_min)
		{
			Mz_min = g_Mz_Raw;
		}

		g_Mx_Offset = (Mx_max + Mx_min) / 2;
		g_My_Offset = (My_max + My_min) / 2;
		g_Mz_Offset = (Mz_max + Mz_min) / 2;

		if (g_Calibration_Done) return 0;

		if (millis() > FXOS_MEAS_CAL_LIM_MS &&
				(Mx_max > (Mx_min + 500)) && (My_max > (My_min + 500)) && (Mz_max > (Mz_min + 500)))
		{
			LOG_INFO("\r\nCalibrate magnetometer successfully!");
			LOG_INFO("Magnetometer offset Mx: %d - My: %d - Mz: %d", g_Mx_Offset, g_My_Offset, g_Mz_Offset);
			LOG_INFO("Magnetometer diff Mx: %d - My: %d - Mz: %d",
					Mx_max - Mx_min, My_max - My_min, Mz_max - Mz_min);

			char buff[128];
			memset(buff, 0, sizeof(buff));
			snprintf(buff, sizeof(buff), "Mag cal OK: Mx: %d - My: %d - Mz: %d",
					Mx_max - Mx_min, My_max - My_min, Mz_max - Mz_min);
			vue.addNotif("Event", buff, 4, eNotificationTypeComplete);

			// save calibration to settings
			sUserParameters *settings = user_settings_get();
			settings->mag_cal.calib[0] = g_Mx_Offset;
			settings->mag_cal.calib[1] = g_My_Offset;
			settings->mag_cal.calib[2] = g_Mz_Offset;
			settings->mag_cal.is_present = 1;

			LOG_WARNING("Saving Mag Cal. to flash...");

			u_settings.writeConfig();

			g_Calibration_Done = true;

		} else if (millis() > FXOS_MEAS_CAL_LIM_MS) {

			LOG_INFO("Mag cal failure");
			LOG_INFO("Magnetometer diff Mx: %d - My: %d - Mz: %d",
					Mx_max - Mx_min, My_max - My_min, Mz_max - Mz_min);
		} else {
			return -1;
		}

	}

	return 0;
}

/*******************************************************************************
 * Code
 ******************************************************************************/

void fxos_calibration_start(void) {
	g_Calibration_Done = false;

	Mx_max = Mx_min = g_Mx_Raw;
	My_max = My_min = g_My_Raw;
	Mz_max = Mz_min = g_Mz_Raw;

	LOG_INFO("Calibrating magnetometer...");

	vue.addNotif("Event", "Calibrating magnetometer...", 4, eNotificationTypeComplete);
}

/**
 *
 */
bool fxos_init(void) {

	static uint8_t p_ans_buffer[2] = {0};

	gpio_set(FXOS_RST);
	delay_ms(1);
	gpio_clear(FXOS_RST);
	delay_ms(10);

#ifndef _DEBUG_TWI

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND fxos_reg1[2] = {WHO_AM_I_REG, CTRL_REG1};

	static nrf_twi_mngr_transfer_t const fxos_init_transfers1[] =
	{
			I2C_READ_REG(FXOS_7BIT_ADDRESS, fxos_reg1  , p_ans_buffer  , 1),
			I2C_READ_REG(FXOS_7BIT_ADDRESS, fxos_reg1+1, p_ans_buffer+1, 1)
	};

	i2c_perform(NULL, fxos_init_transfers1, sizeof(fxos_init_transfers1) / sizeof(fxos_init_transfers1[0]), NULL);

#else

	FXOS_ReadReg(nullptr, WHO_AM_I_REG, p_ans_buffer, 1);

#endif

	if (p_ans_buffer[0] != kFXOS_WHO_AM_I_Device_ID)
	{
		LOG_ERROR("FXOS absent WHO 0x%X", p_ans_buffer[0]);
		return kStatus_Fail;
	} else {
		LOG_INFO("FXOS dev ID: 0x%02X", p_ans_buffer[0]);
		LOG_INFO("FXOS CTRL1 : 0x%02X", p_ans_buffer[1]);
	}

#ifdef _DEBUG_TWI

	/* setup auto sleep with FFMT trigger */
	/* go to standby */
	if(FXOS_ReadReg(nullptr, CTRL_REG1, p_ans_buffer, 1) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}

	if(FXOS_WriteReg(nullptr, CTRL_REG1, p_ans_buffer[0] & (uint8_t)~ACTIVE_MASK) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}

	/* Read again to make sure we are in standby mode. */
	if(FXOS_ReadReg(nullptr, CTRL_REG1, p_ans_buffer, 1) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}
	if ((p_ans_buffer[0] & ACTIVE_MASK) == ACTIVE_MASK)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}


	/* Disable the FIFO */
	if(FXOS_WriteReg(nullptr, F_SETUP_REG, F_MODE_DISABLED) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}

#ifdef LPSLEEP_HIRES
	/* enable auto-sleep, low power in sleep, high res in wake */
	if(FXOS_WriteReg(fxos_handle, CTRL_REG2, SLPE_MASK | SMOD_LOW_POWER | MOD_HIGH_RES) != kStatus_Success)
	{
		return kStatus_Fail;
	}
#else
	/* high res in wake */
	if(FXOS_WriteReg(nullptr, CTRL_REG2, MOD_LOW_NOISE) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}

#endif

	/* set up Mag OSR and Hybrid mode using M_CTRL_REG1, use default for Acc */
	if(FXOS_WriteReg(nullptr, M_CTRL_REG1, (M_RST_MASK | M_OSR_MASK | M_HMS_MASK)) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}

	/* Enable hyrid mode auto increment using M_CTRL_REG2 */
	if(FXOS_WriteReg(nullptr, M_CTRL_REG2, (M_HYB_AUTOINC_MASK)) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}

#ifdef EN_FFMT
	/* enable FFMT for motion detect for X and Y axes, latch enable */
	if(FXOS_WriteReg(fxos_handle, FF_MT_CFG_REG, XEFE_MASK | YEFE_MASK | ELE_MASK | OAE_MASK) != kStatus_Success)
	{
		return kStatus_Fail;
	}
#endif

#ifdef SET_THRESHOLD
	/* set threshold to about 0.25g */
	if(FXOS_WriteReg(fxos_handle, FT_MT_THS_REG, 0x04) != kStatus_Success)
	{
		return kStatus_Fail;
	}
#endif



#ifdef SET_DEBOUNCE
	/* set debounce to zero */
	if(FXOS_WriteReg(fxos_handle, FF_MT_COUNT_REG, 0x00) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}
#endif

#ifdef EN_AUTO_SLEEP
	/* set auto-sleep wait period to 5s (=5/0.64=~8) */
	if(FXOS_WriteReg(fxos_handle, ASLP_COUNT_REG, 8) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}
#endif
	/* default set to 4g mode */
	if(FXOS_WriteReg(nullptr, XYZ_DATA_CFG_REG, FULL_SCALE_4G) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}

	/* Use LPF */
	if(FXOS_WriteReg(nullptr, HP_FILTER_CUTOFF_REG, PULSE_LPF_EN_MASK) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}


#ifdef EN_INTERRUPTS
	/* enable data-ready, auto-sleep and motion detection interrupts */
	/* FXOS1_WriteRegister(CTRL_REG4, INT_EN_DRDY_MASK | INT_EN_ASLP_MASK | INT_EN_FF_MT_MASK); */
	if(FXOS_WriteReg(fxos_handle, CTRL_REG4, 0x0) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}
	/* route data-ready interrupts to INT1, others INT2 (default) */
	if(FXOS_WriteReg(fxos_handle, CTRL_REG5, INT_CFG_DRDY_MASK) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}
	/* enable ffmt as a wake-up source */
	if(FXOS_WriteReg(fxos_handle, CTRL_REG3, WAKE_FF_MT_MASK) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}
	/* finally activate accel_device with ASLP ODR=0.8Hz, ODR=100Hz, FSR=2g */
	if(FXOS_WriteReg(fxos_handle, CTRL_REG1, HYB_ASLP_RATE_0_8HZ | HYB_DATA_RATE_100HZ | ACTIVE_MASK) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}
#else
	/* Setup the ODR for 50 Hz and activate the accelerometer */
	if(FXOS_WriteReg(nullptr, CTRL_REG1, (HYB_DATA_RATE_25HZ | ACTIVE_MASK)) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}
#endif

	/* Read Control register again to ensure we are in active mode */
	if(FXOS_ReadReg(nullptr, CTRL_REG1, p_ans_buffer, 1) != kStatus_Success)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}

	if ((p_ans_buffer[0] & ACTIVE_MASK) != ACTIVE_MASK)
	{
		APP_ERROR_CHECK(0x7);
		return kStatus_Fail;
	}

	LOG_INFO("FXOS CTRL1 : 0x%02X", p_ans_buffer[0]);
#else
	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND fxos_config[][2] =
	{
			/* Put in standby */
			{CTRL_REG1, 0x00},
			/* Disable the FIFO */
			{F_SETUP_REG, F_MODE_DISABLED},
#ifdef LPSLEEP_HIRES
			/* enable auto-sleep, low power in sleep, high res in wake */
			{CTRL_REG2, SLPE_MASK | SMOD_LOW_POWER | MOD_HIGH_RES},
#else
			/* Mode low noise low power */
			{CTRL_REG2, MOD_LOW_NOISE},
#endif
			/* set up Mag OSR and Hybrid mode using M_CTRL_REG1, use default for Acc */
			{M_CTRL_REG1, (M_RST_MASK | M_OSR_MASK | M_HMS_MASK)},
			/* Enable hybrid mode auto increment using M_CTRL_REG2 */
			{M_CTRL_REG2, (M_HYB_AUTOINC_MASK)},

#ifdef EN_FFMT
			/* enable FFMT for motion detect for X and Y axes, latch enable */
			{FF_MT_CFG_REG, (XEFE_MASK | YEFE_MASK | ELE_MASK | OAE_MASK)},
#endif

#ifdef SET_THRESHOLD
			/* set threshold to about 0.25g */
			{FT_MT_THS_REG, 0x04},
#endif

#ifdef SET_DEBOUNCE
			/* set debounce to zero */
			{FF_MT_COUNT_REG, 0x00},
#endif

#ifdef EN_AUTO_SLEEP
			/* set auto-sleep wait period to 5s (=5/0.64=~8) */
			{ASLP_COUNT_REG, 8},
#endif
			/* default set to 4g mode */
			{XYZ_DATA_CFG_REG, FULL_SCALE_4G},

#ifdef EN_INTERRUPTS
			/* enable data-ready, auto-sleep and motion detection interrupts */
			/* FXOS1_WriteRegister(CTRL_REG4, INT_EN_DRDY_MASK | INT_EN_ASLP_MASK | INT_EN_FF_MT_MASK); */
			{CTRL_REG4, INT_EN_DRDY_MASK},
			/* route data-ready interrupts to INT1, others INT2 (default) */
			{CTRL_REG5, INT_CFG_DRDY_MASK},
#endif

			/* HPF bypassed */
			{HP_FILTER_CUTOFF_REG, PULSE_HPF_BYP_MASK },

			/* Setup the ODR activate the accelerometer */
			{CTRL_REG1, (HYB_DATA_RATE_50HZ | ACTIVE_MASK)},
	};

	uint16_t cur_ind = 0;
	static nrf_twi_mngr_transfer_t const fxos_init_transfers2[] =
	{
			/* Put in standby */
			I2C_WRITE(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], 2),
			/* Disable the FIFO */
			I2C_WRITE(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], 2),
#ifdef LPSLEEP_HIRES
			/* enable auto-sleep, low power in sleep, high res in wake */
			I2C_WRITE(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], 2),
#else
			I2C_WRITE(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], 2),
#endif
			/* set up Mag OSR and Hybrid mode using M_CTRL_REG1, use default for Acc */
			I2C_WRITE(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], 2),
			/* Enable hyrid mode auto increment using M_CTRL_REG2 */
			I2C_WRITE(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], 2),

#ifdef EN_FFMT
			/* enable FFMT for motion detect for X and Y axes, latch enable */
			I2C_WRITE(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], 2),
#endif

#ifdef SET_THRESHOLD
			/* set threshold to about 0.25g */
			I2C_WRITE(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], 2),
#endif

#ifdef SET_DEBOUNCE
			/* set debounce to zero */
			I2C_WRITE(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], 2),
#endif

#ifdef EN_AUTO_SLEEP
			/* set auto-sleep wait period to 5s (=5/0.64=~8) */
			I2C_WRITE(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], 2),
#endif
			/* default set to 4g mode */
			I2C_WRITE(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], 2),

#ifdef EN_INTERRUPTS
			/* enable data-ready, auto-sleep and motion detection interrupts */
			/* FXOS1_WriteRegister(CTRL_REG4, INT_EN_DRDY_MASK | INT_EN_ASLP_MASK | INT_EN_FF_MT_MASK); */
			I2C_WRITE(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], 2),
			/* route data-ready interrupts to INT1, others INT2 (default) */
			I2C_WRITE(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], 2),
#endif

			/* Use LPF */
			I2C_WRITE(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], 2),

			/* Setup the ODR and activate the accelerometer */
			I2C_WRITE(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], 2)
	};

	LOG_DEBUG("FXOS Xfer 1 0x%X", fxos_init_transfers2[0].p_data[0]);
	LOG_DEBUG("FXOS Xfer 2 0x%X", fxos_init_transfers2[1].p_data[0]);
	LOG_DEBUG("FXOS Xfer 3 0x%X", fxos_init_transfers2[2].p_data[0]);

	i2c_perform(NULL, fxos_init_transfers2, sizeof(fxos_init_transfers2) / sizeof(fxos_init_transfers2[0]), NULL);
#endif


#ifdef EN_INTERRUPTS

#ifdef _DEBUG_TWI
#error "Incompatible SW modes"
#endif

	// Configure INT pin
	nrfx_gpiote_in_config_t in_config;
	in_config.is_watcher = true;
	in_config.hi_accuracy = true;
	in_config.skip_gpio_setup = false;
	in_config.pull = NRF_GPIO_PIN_PULLUP;
	in_config.sense = NRF_GPIOTE_POLARITY_HITOLO;

	ret_code_t err_code = nrfx_gpiote_in_init(FXOS_INT1, &in_config, _int_handler);
	APP_ERROR_CHECK(err_code);

	nrfx_gpiote_in_event_enable(FXOS_INT1, true);

	nrf_gpio_cfg_sense_input(FXOS_INT1, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);

#endif


	return kStatus_Success;
}

/**
 *
 * @param g_fxosHandle
 * @return
 */
void fxos_tasks(void)
{
	uint16_t i = 0;
//	float sinAngle = 0;
//	float cosAngle = 0;
//	float Bx = 0;
//	float By = 0;

	float g_Ax = 0;
	float g_Ay = 0;
	float g_Az = 0;

	float g_Mx = 0;
	float g_My = 0;
//	float g_Mz = 0;

	static float g_Mx_LP = 0;
	static float g_My_LP = 0;
	static float g_Mz_LP = 0;

	static bool g_FirstRun = true;

	g_Ax = 0;
	g_Ay = 0;
	g_Az = 0;

	/* Read buffered sensor data */
	for (i = 0; i < MAX_ACCEL_AVG_COUNT; i++)
	{
		g_Ax += (float)g_Ax_buff[i];
		g_Ay += (float)g_Ay_buff[i];
		g_Az += (float)g_Az_buff[i];

		g_Mx_Raw += (float)g_Mx_buff[i];
		g_My_Raw += (float)g_My_buff[i];
		g_Mz_Raw += (float)g_Mz_buff[i];
	}

	g_Ax /= MAX_ACCEL_AVG_COUNT;
	g_Ay /= MAX_ACCEL_AVG_COUNT;
	g_Az /= MAX_ACCEL_AVG_COUNT;

	g_Mx_Raw /= MAX_ACCEL_AVG_COUNT;
	g_My_Raw /= MAX_ACCEL_AVG_COUNT;
	g_Mz_Raw /= MAX_ACCEL_AVG_COUNT;

	LOG_DEBUG("Accel: %d %d %d", (int)g_Ax, (int)g_Ay, (int)g_Az);

	if (g_FirstRun) {

		if(g_sensorRange == 0x00)
		{
			g_dataScale = 2U;
		}
		else if(g_sensorRange == 0x01)
		{
			g_dataScale = 4U;
		}
		else if(g_sensorRange == 0x10)
		{
			g_dataScale = 8U;
		}
		else
		{
		}

		if (u_settings.isConfigValid()) {

			sMagCal &mag_cal = u_settings.getMagCal();
			// check if we have a previous calibration
			if (mag_cal.is_present) {

				g_Mx_Offset = mag_cal.calib[0];
				g_My_Offset = mag_cal.calib[1];
				g_Mz_Offset = mag_cal.calib[2];

				g_Calibration_Done = true;

				LOG_INFO("Magnetometer calibration found");

				//vue.addNotif("Event", "Magnetometer calibration found", 4, eNotificationTypeComplete);

			}

		}



	}

	if (!g_Calibration_Done) {
		Magnetometer_Calibrate_Task();
	}

	if(g_FirstRun) {
		g_Mx_LP = g_Mx_Raw;
		g_My_LP = g_My_Raw;
		g_Mz_LP = g_Mz_Raw;
	}

	// filtre ?
	g_Mx_LP += (g_Mx_Raw - g_Mx_LP) * FXOS_MAG_FILTER_COEFF;
	g_My_LP += (g_My_Raw - g_My_LP) * FXOS_MAG_FILTER_COEFF;
	g_Mz_LP += (g_Mz_Raw - g_Mz_LP) * FXOS_MAG_FILTER_COEFF;

	/* Calculate magnetometer values */
	g_Mx = g_Mx_LP - g_Mx_Offset;
	g_My = g_My_LP - g_My_Offset;
	g_Mz = g_Mz_LP - g_Mz_Offset;

	LOG_DEBUG("Mag. comp.: %d %d %d", (int)g_Mx, (int)g_My, (int)g_Mz);

	/* Calculate roll angle g_Roll (-180deg, 180deg) and sin, cos */
#if defined( PROTO_V11)
	g_Roll = atan2f(g_Ax, g_Az);
//	sinAngle = sinf(g_Roll);
//	cosAngle = cosf(g_Roll);
#else
	g_Roll = atan2f(g_Ay, g_Az);
//	sinAngle = sinf(g_Roll);
//	cosAngle = cosf(g_Roll);
#endif

	LOG_DEBUG("Roll: %d deg/10", (int)(g_Roll*RadToDeg*10.f));

	/* De-rotate by roll angle g_Roll */
	//By = g_My * cosAngle - g_Mz * sinAngle;
	//g_Mz = g_Mz * cosAngle + g_My * sinAngle;
	//g_Az = g_Ay * sinAngle + g_Az * cosAngle;

	/* Calculate pitch angle g_Pitch and sin, cos*/
#if defined( PROTO_V11)
	g_Pitch = -atan2f( g_Ay , -g_Az);
//	sinAngle = sinf(g_Pitch);
//	cosAngle = cosf(g_Pitch);
#else
	g_Pitch = atan2f( g_Ax , g_Az);
//	sinAngle = sinf(g_Pitch);
//	cosAngle = cosf(g_Pitch);
#endif

	LOG_INFO("Pitch: %d deg/10", (int)(g_Pitch*RadToDeg*10.f));

	/* De-rotate by pitch angle g_Pitch */
	//Bx = g_Mx * cosAngle + g_Mz * sinAngle;

	/* Calculate yaw = ecompass angle psi (-180deg, 180deg) */
#if defined( PROTO_V11)
	g_Yaw = atan2f(-g_My, g_Mx);
#else
	g_Yaw = atan2f(-g_My, g_Mx);
#endif
	if(g_FirstRun)
	{
		g_Yaw_LP = g_Yaw;
	}
	g_Yaw_LP += (g_Yaw - g_Yaw_LP) * FXOS_MAG_FILTER_COEFF;

	LOG_DEBUG("Compass Angle raw   : %d  ", (int)(g_Yaw*RadToDeg*10.f));
	LOG_DEBUG("Compass Angle filtered: %d", (int)(g_Yaw_LP*RadToDeg*10.f));

	g_FirstRun = false;

	// store pitch
	int16_t integ_pitch = (int16_t)((g_Pitch + 1.57f) * 100.f);
	uint16_t u_integ_pitch = (uint16_t)integ_pitch;

	if (m_pitch_buffer.isFull()) {
		m_pitch_buffer.popLast();
	}
	m_pitch_buffer.add(&u_integ_pitch);

}

bool fxos_get_yaw(float &yaw_rad) {

	yaw_rad = g_Yaw;

	return true;
}

bool fxos_get_pitch(float &pitch_rad) {

	pitch_rad = g_Pitch;

	return true;
}

tHistoValue fxos_histo_read(uint16_t ind_) {

	tHistoValue *p_ret_val = m_pitch_buffer.get(ind_);

	ASSERT(p_ret_val);

	tHistoValue ret_val = p_ret_val[0];

	return ret_val;
}

uint16_t fxos_histo_size(void) {

	return m_pitch_buffer.size();
}
