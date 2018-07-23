/*
 * FXOS.cpp
 *
 *  Created on: 12 nov. 2017
 *      Author: Vincent
 */

#include "i2c.h"
#include "fsl_fxos.h"
#include "parameters.h"
#include "segger_wrapper.h"
#include <sensors/fxos.h>
#include <math.h>
#include <millis.h>

#define kStatus_Fail    1
#define kStatus_Success 0



ret_code_t FXOS_ReadReg(fxos_handle_t *handle, uint8_t reg, uint8_t *val, uint8_t bytesNumber)
{
	ret_code_t status = NRF_SUCCESS;

#ifdef _DEBUG_TWI
	if (!i2c_write8(FXOS_7BIT_ADDRESS, reg)) {
		LOG_ERROR("FXOS Error 1");
		status = 1;
	}

	// repeated start
	if (!i2c_read_n(FXOS_7BIT_ADDRESS, val, bytesNumber)) {
		LOG_ERROR("FXOS Error 2");
		status += 1;
	}
#else

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

#define MAX_ACCEL_AVG_COUNT 25U

/* multiplicative conversion constants */
#define DegToRad 0.017453292
#define RadToDeg 57.295779

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void Sensor_ReadData(fxos_handle_t *g_fxosHandle, int16_t *Ax, int16_t *Ay, int16_t *Az, int16_t *Mx, int16_t *My, int16_t *Mz);

static void Magnetometer_Calibrate(fxos_handle_t *g_fxosHandle);

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile uint16_t SampleEventFlag;
uint8_t g_sensorRange = FULL_SCALE_2G;
uint8_t g_dataScale = 0;

int16_t g_Ax_Raw = 0;
int16_t g_Ay_Raw = 0;
int16_t g_Az_Raw = 0;

double g_Ax = 0;
double g_Ay = 0;
double g_Az = 0;

int16_t g_Ax_buff[MAX_ACCEL_AVG_COUNT] = {0};
int16_t g_Ay_buff[MAX_ACCEL_AVG_COUNT] = {0};
int16_t g_Az_buff[MAX_ACCEL_AVG_COUNT] = {0};

int16_t g_Mx_Raw = 0;
int16_t g_My_Raw = 0;
int16_t g_Mz_Raw = 0;

int16_t g_Mx_Offset = 0;
int16_t g_My_Offset = 0;
int16_t g_Mz_Offset = 0;

double g_Mx = 0;
double g_My = 0;
double g_Mz = 0;

double g_Mx_LP = 0;
double g_My_LP = 0;
double g_Mz_LP = 0;

double g_Yaw = 0;
double g_Yaw_LP = 0;
double g_Pitch = 0;
double g_Roll = 0;

bool g_FirstRun = true;

/*******************************************************************************
 * Code
 ******************************************************************************/

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
static void Sensor_ReadData(fxos_handle_t *g_fxosHandle, int16_t *Ax, int16_t *Ay, int16_t *Az, int16_t *Mx, int16_t *My, int16_t *Mz)
{
	fxos_data_t fxos_data;
#ifdef _DEBUG_TWI
	if (FXOS_ReadReg(nullptr, M_OUT_X_MSB_REG, &fxos_data.magXMSB, 6)) {
		return;
	}
	if (FXOS_ReadReg(nullptr, OUT_X_MSB_REG, &fxos_data.accelXMSB, 6)) {
		return;
	}
#else
	ASSERT(g_fxosHandle);
	memcpy(&fxos_data.accelXMSB, g_fxosHandle->acc_buffer, 6);
	memcpy(&fxos_data.magXMSB  , g_fxosHandle->mag_buffer, 6);
#endif
	/* Get the accel data from the sensor data structure in 14 bit left format data*/
	*Ax = (int16_t)((uint16_t)((uint16_t)fxos_data.accelXMSB << 8) | (uint16_t)fxos_data.accelXLSB)/4U;
	*Ay = (int16_t)((uint16_t)((uint16_t)fxos_data.accelYMSB << 8) | (uint16_t)fxos_data.accelYLSB)/4U;
	*Az = (int16_t)((uint16_t)((uint16_t)fxos_data.accelZMSB << 8) | (uint16_t)fxos_data.accelZLSB)/4U;

	*Ax *= g_dataScale;
	*Ay *= g_dataScale;
	*Az *= g_dataScale;
	*Mx = (int16_t)((uint16_t)((uint16_t)fxos_data.magXMSB << 8) | (uint16_t)fxos_data.magXLSB);
	*My = (int16_t)((uint16_t)((uint16_t)fxos_data.magYMSB << 8) | (uint16_t)fxos_data.magYLSB);
	*Mz = (int16_t)((uint16_t)((uint16_t)fxos_data.magZMSB << 8) | (uint16_t)fxos_data.magZLSB);

}

static void Magnetometer_Calibrate(fxos_handle_t *g_fxosHandle)
{
	int16_t Mx_max = 0;
	int16_t My_max = 0;
	int16_t Mz_max = 0;
	int16_t Mx_min = 0;
	int16_t My_min = 0;
	int16_t Mz_min = 0;

	uint32_t times = 0;
	LOG_INFO("\r\nCalibrating magnetometer...");
	LOG_INFO("\r\n3");
	delay_ms(1000);
	LOG_INFO("\r\n2");
	delay_ms(1000);
	LOG_INFO("\r\n1");
	delay_ms(1000);

	while (times < 400)
	{
		Sensor_ReadData(g_fxosHandle, &g_Ax_Raw, &g_Ay_Raw, &g_Az_Raw, &g_Mx_Raw, &g_My_Raw, &g_Mz_Raw);
		if (times == 0)
		{
			Mx_max = Mx_min = g_Mx_Raw;
			My_max = My_min = g_My_Raw;
			Mz_max = Mz_min = g_Mz_Raw;
		}
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
		if ((times % (8000 / 50)) == 0)
		{
			if ((Mx_max > (Mx_min + 500)) && (My_max > (My_min + 500)) && (Mz_max > (Mz_min + 500)))
			{
				g_Mx_Offset = (Mx_max + Mx_min) / 2;
				g_My_Offset = (My_max + My_min) / 2;
				g_Mz_Offset = (Mz_max + Mz_min) / 2;
				LOG_INFO("\r\nCalibrate magnetometer successfully!");
				LOG_INFO("\r\nMagnetometer offset Mx: %d - My: %d - Mz: %d \r\n", g_Mx_Offset, g_My_Offset, g_Mz_Offset);
				break;
			}
		}
		times++;
		delay_ms(50);
	}
}

/**
 *
 */
bool fxos_init(void) {

	static uint8_t p_ans_buffer[2] = {0};

#ifndef _DEBUG_TWI

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND fxos_reg1[2] = {CTRL_REG1, WHO_AM_I_REG};

	static nrf_twi_mngr_transfer_t const fxos_init_transfers1[] =
	{
			I2C_READ_REG_REP_START(FXOS_7BIT_ADDRESS, fxos_reg1  , p_ans_buffer  , 1),
			I2C_READ_REG_REP_START(FXOS_7BIT_ADDRESS, fxos_reg1+1, p_ans_buffer+1, 1)
	};

	i2c_perform(NULL, fxos_init_transfers1, sizeof(fxos_init_transfers1) / sizeof(fxos_init_transfers1[0]), NULL);

#else

	if(FXOS_ReadReg(nullptr, WHO_AM_I_REG, p_ans_buffer, 1) != kStatus_Success)
	{
		return kStatus_Fail;
	}

#endif

	if (p_ans_buffer[0] != kFXOS_WHO_AM_I_Device_ID)
	{
		LOG_ERROR("FXOS absent");
		return kStatus_Fail;
	}

#ifdef _DEBUG_TWI

	/* setup auto sleep with FFMT trigger */
	/* go to standby */
	if(FXOS_ReadReg(nullptr, CTRL_REG1, p_ans_buffer, 1) != kStatus_Success)
	{
		return kStatus_Fail;
	}

	if(FXOS_WriteReg(nullptr, CTRL_REG1, p_ans_buffer[0] & (uint8_t)~ACTIVE_MASK) != kStatus_Success)
	{
		return kStatus_Fail;
	}

	/* Read again to make sure we are in standby mode. */
	if(FXOS_ReadReg(nullptr, CTRL_REG1, p_ans_buffer, 1) != kStatus_Success)
	{
		return kStatus_Fail;
	}
	if ((p_ans_buffer[0] & ACTIVE_MASK) == ACTIVE_MASK)
	{
		return kStatus_Fail;
	}


	/* Disable the FIFO */
	if(FXOS_WriteReg(nullptr, F_SETUP_REG, F_MODE_DISABLED) != kStatus_Success)
	{
		return kStatus_Fail;
	}

#ifdef LPSLEEP_HIRES
	/* enable auto-sleep, low power in sleep, high res in wake */
	if(FXOS_WriteReg(fxos_handle, CTRL_REG2, SLPE_MASK | SMOD_LOW_POWER | MOD_HIGH_RES) != kStatus_Success)
	{
		return kStatus_Fail;
	}
#else
	/* enable auto-sleep, low power in sleep, high res in wake */
	if(FXOS_WriteReg(nullptr, CTRL_REG2, MOD_HIGH_RES) != kStatus_Success)
	{
		return kStatus_Fail;
	}

#endif

	/* set up Mag OSR and Hybrid mode using M_CTRL_REG1, use default for Acc */
	if(FXOS_WriteReg(nullptr, M_CTRL_REG1, (M_RST_MASK | M_OSR_MASK | M_HMS_MASK)) != kStatus_Success)
	{
		return kStatus_Fail;
	}

	/* Enable hyrid mode auto increment using M_CTRL_REG2 */
	if(FXOS_WriteReg(nullptr, M_CTRL_REG2, (M_HYB_AUTOINC_MASK)) != kStatus_Success)
	{
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
		return kStatus_Fail;
	}
#endif

#ifdef EN_AUTO_SLEEP
	/* set auto-sleep wait period to 5s (=5/0.64=~8) */
	if(FXOS_WriteReg(fxos_handle, ASLP_COUNT_REG, 8) != kStatus_Success)
	{
		return kStatus_Fail;
	}
#endif
	/* default set to 4g mode */
	if(FXOS_WriteReg(nullptr, XYZ_DATA_CFG_REG, FULL_SCALE_4G) != kStatus_Success)
	{
		return kStatus_Fail;
	}


#ifdef EN_INTERRUPTS
	/* enable data-ready, auto-sleep and motion detection interrupts */
	/* FXOS1_WriteRegister(CTRL_REG4, INT_EN_DRDY_MASK | INT_EN_ASLP_MASK | INT_EN_FF_MT_MASK); */
	if(FXOS_WriteReg(fxos_handle, CTRL_REG4, 0x0) != kStatus_Success)
	{
		return kStatus_Fail;
	}
	/* route data-ready interrupts to INT1, others INT2 (default) */
	if(FXOS_WriteReg(fxos_handle, CTRL_REG5, INT_CFG_DRDY_MASK) != kStatus_Success)
	{
		return kStatus_Fail;
	}
	/* enable ffmt as a wake-up source */
	if(FXOS_WriteReg(fxos_handle, CTRL_REG3, WAKE_FF_MT_MASK) != kStatus_Success)
	{
		return kStatus_Fail;
	}
	/* finally activate accel_device with ASLP ODR=0.8Hz, ODR=100Hz, FSR=2g */
	if(FXOS_WriteReg(fxos_handle, CTRL_REG1, HYB_ASLP_RATE_0_8HZ | HYB_DATA_RATE_100HZ | ACTIVE_MASK) != kStatus_Success)
	{
		return kStatus_Fail;
	}
#else
	/* Setup the ODR for 50 Hz and activate the accelerometer */
	if(FXOS_WriteReg(nullptr, CTRL_REG1, (HYB_DATA_RATE_200HZ | ACTIVE_MASK)) != kStatus_Success)
	{
		return kStatus_Fail;
	}
#endif

	/* Read Control register again to ensure we are in active mode */
	if(FXOS_ReadReg(nullptr, CTRL_REG1, p_ans_buffer, 1) != kStatus_Success)
	{
		return kStatus_Fail;
	}

	if ((p_ans_buffer[0] & ACTIVE_MASK) != ACTIVE_MASK)
	{
		return kStatus_Fail;
	}
#else
	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND fxos_config[][2] =
	{
			/* Put in standby */
			FXOS_STANDBY_REGS(p_ans_buffer[0]),
			/* Disable the FIFO */
			{F_SETUP_REG, F_MODE_DISABLED},
#ifdef LPSLEEP_HIRES
			/* enable auto-sleep, low power in sleep, high res in wake */
			{CTRL_REG2, SLPE_MASK | SMOD_LOW_POWER | MOD_HIGH_RES},
#else
			{CTRL_REG2, MOD_HIGH_RES},
#endif
			/* set up Mag OSR and Hybrid mode using M_CTRL_REG1, use default for Acc */
			{M_CTRL_REG1, (M_RST_MASK | M_OSR_MASK | M_HMS_MASK)},
			/* Enable hyrid mode auto increment using M_CTRL_REG2 */
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

			/* Setup the ODR for 50 Hz and activate the accelerometer */
			{CTRL_REG1, (HYB_DATA_RATE_200HZ | ACTIVE_MASK)},
	};

	uint16_t cur_ind = 0;
	static nrf_twi_mngr_transfer_t const fxos_init_transfers2[] =
	{
			/* Put in standby */
			FXOS_STANDBY(fxos_config[cur_ind++], 1),
			/* Disable the FIFO */
			I2C_WRITE_REG(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], &fxos_config[cur_ind++][1], 1),
#ifdef LPSLEEP_HIRES
			/* enable auto-sleep, low power in sleep, high res in wake */
			I2C_WRITE_REG(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], &fxos_config[cur_ind++][1], 1),
#else
			I2C_WRITE_REG(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], &fxos_config[cur_ind++][1], 1),
#endif
			/* set up Mag OSR and Hybrid mode using M_CTRL_REG1, use default for Acc */
			I2C_WRITE_REG(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], &fxos_config[cur_ind++][1], 1),
			/* Enable hyrid mode auto increment using M_CTRL_REG2 */
			I2C_WRITE_REG(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], &fxos_config[cur_ind++][1], 1),

#ifdef EN_FFMT
			/* enable FFMT for motion detect for X and Y axes, latch enable */
			I2C_WRITE_REG(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], &fxos_config[cur_ind++][1], 1),
#endif

#ifdef SET_THRESHOLD
			/* set threshold to about 0.25g */
			I2C_WRITE_REG(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], &fxos_config[cur_ind++][1], 1),
#endif

#ifdef SET_DEBOUNCE
			/* set debounce to zero */
#endif

#ifdef EN_AUTO_SLEEP
			/* set auto-sleep wait period to 5s (=5/0.64=~8) */
			I2C_WRITE_REG(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], &fxos_config[cur_ind++][1], 1),
#endif
			/* default set to 4g mode */
			I2C_WRITE_REG(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], &fxos_config[cur_ind++][1], 1),

			/* Setup the ODR for 50 Hz and activate the accelerometer */
			I2C_WRITE_REG(FXOS_7BIT_ADDRESS, &fxos_config[cur_ind++][0], &fxos_config[cur_ind++][1], 1),
	};

	i2c_perform(NULL, fxos_init_transfers2, sizeof(fxos_init_transfers2) / sizeof(fxos_init_transfers2[0]), NULL);
#endif
	return kStatus_Success;
}

/**
 *
 * @param g_fxosHandle
 * @return
 */
void fxos_tasks(fxos_handle_t *g_fxosHandle)
{
	uint16_t i = 0;
	double sinAngle = 0;
	double cosAngle = 0;
	double Bx = 0;
	double By = 0;

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

		// TODO if (g_fxosHandle) Magnetometer_Calibrate(g_fxosHandle);
	}

	SampleEventFlag = 0;
	g_Ax_Raw = 0;
	g_Ay_Raw = 0;
	g_Az_Raw = 0;
	g_Ax = 0;
	g_Ay = 0;
	g_Az = 0;
	g_Mx_Raw = 0;
	g_My_Raw = 0;
	g_Mz_Raw = 0;
	g_Mx = 0;
	g_My = 0;
	g_Mz = 0;

	/* Read sensor data */
	Sensor_ReadData(g_fxosHandle, &g_Ax_Raw, &g_Ay_Raw, &g_Az_Raw, &g_Mx_Raw, &g_My_Raw, &g_Mz_Raw);

	/* Average accel value */
	for (i = 1; i < MAX_ACCEL_AVG_COUNT; i++)
	{
		g_Ax_buff[i] = g_Ax_buff[i - 1];
		g_Ay_buff[i] = g_Ay_buff[i - 1];
		g_Az_buff[i] = g_Az_buff[i - 1];
	}

	g_Ax_buff[0] = g_Ax_Raw;
	g_Ay_buff[0] = g_Ay_Raw;
	g_Az_buff[0] = g_Az_Raw;

	for (i = 0; i < MAX_ACCEL_AVG_COUNT; i++)
	{
		g_Ax += (double)g_Ax_buff[i];
		g_Ay += (double)g_Ay_buff[i];
		g_Az += (double)g_Az_buff[i];
	}

	g_Ax /= MAX_ACCEL_AVG_COUNT;
	g_Ay /= MAX_ACCEL_AVG_COUNT;
	g_Az /= MAX_ACCEL_AVG_COUNT;

	LOG_INFO("Accel: %d %d %d", (int)g_Ax, (int)g_Ay, (int)g_Az);

	if(g_FirstRun)
	{
		g_Mx_LP = g_Mx_Raw;
		g_My_LP = g_My_Raw;
		g_Mz_LP = g_Mz_Raw;
	}

	g_Mx_LP += ((double)g_Mx_Raw - g_Mx_LP) * 0.01;
	g_My_LP += ((double)g_My_Raw - g_My_LP) * 0.01;
	g_Mz_LP += ((double)g_Mz_Raw - g_Mz_LP) * 0.01;

	/* Calculate magnetometer values */
	g_Mx = g_Mx_LP - g_Mx_Offset;
	g_My = g_My_LP - g_My_Offset;
	g_Mz = g_Mz_LP - g_Mz_Offset;

	/* Calculate roll angle g_Roll (-180deg, 180deg) and sin, cos */
	g_Roll = atan2(g_Ay, g_Az) * RadToDeg;
	sinAngle = sin(g_Roll * DegToRad);
	cosAngle = cos(g_Roll * DegToRad);

	/* De-rotate by roll angle g_Roll */
	By = g_My * cosAngle - g_Mz * sinAngle;
	g_Mz = g_Mz * cosAngle + g_My * sinAngle;
	g_Az = g_Ay * sinAngle + g_Az * cosAngle;

	/* Calculate pitch angle g_Pitch (-90deg, 90deg) and sin, cos*/
	g_Pitch = atan2(-g_Ax , g_Az) * RadToDeg;
	sinAngle = sin(g_Pitch * DegToRad);
	cosAngle = cos(g_Pitch * DegToRad);

	/* De-rotate by pitch angle g_Pitch */
	Bx = g_Mx * cosAngle + g_Mz * sinAngle;

	/* Calculate yaw = ecompass angle psi (-180deg, 180deg) */
	g_Yaw = atan2(-By, Bx) * RadToDeg;
	if(g_FirstRun)
	{
		g_Yaw_LP = g_Yaw;
		g_FirstRun = false;
	}

	g_Yaw_LP += (g_Yaw - g_Yaw_LP) * 0.01;

	LOG_INFO("Compass Angle: %d", (int)g_Yaw);

}
