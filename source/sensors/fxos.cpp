/*
 * FXOS.cpp
 *
 *  Created on: 12 nov. 2017
 *      Author: Vincent
 */

#include "i2c.h"
#include "fsl_fxos.h"
#include "segger_wrapper.h"
#include <sensors/fxos.h>
#include <math.h>
#include <millis.h>


ret_code_t FXOS_ReadReg(fxos_handle_t *handle, uint8_t reg, uint8_t *val, uint8_t bytesNumber)
{
	ret_code_t status = NRF_SUCCESS;
	return status;
}

ret_code_t FXOS_WriteReg(fxos_handle_t *handle, uint8_t reg, uint8_t val)
{
	ret_code_t status = NRF_SUCCESS;
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
uint8_t g_sensorRange = 0;
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

	memcpy(&fxos_data.accelXMSB, g_fxosHandle->acc_buffer, 6);
	memcpy(&fxos_data.magXMSB  , g_fxosHandle->mag_buffer, 6);

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
void fxos_init(void) {

	static uint8_t tmp[2] = {0,0};

//	if(FXOS_ReadReg(fxos_handle, WHO_AM_I_REG, tmp, 1) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//
//	if (tmp[0] != kFXOS_WHO_AM_I_Device_ID)
//	{
//		return kStatus_Fail;
//	}

	/* setup auto sleep with FFMT trigger */
	/* go to standby */
//	if(FXOS_ReadReg(fxos_handle, CTRL_REG1, tmp, 1) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//
//	if(FXOS_WriteReg(fxos_handle, CTRL_REG1, tmp[0] & (uint8_t)~ACTIVE_MASK) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//
//	/* Read again to make sure we are in standby mode. */
//	if(FXOS_ReadReg(fxos_handle, CTRL_REG1, tmp, 1) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//	if ((tmp[0] & ACTIVE_MASK) == ACTIVE_MASK)
//	{
//		return kStatus_Fail;
//	}


//	/* Disable the FIFO */
//	if(FXOS_WriteReg(fxos_handle, F_SETUP_REG, F_MODE_DISABLED) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//
//#ifdef LPSLEEP_HIRES
//	/* enable auto-sleep, low power in sleep, high res in wake */
//	if(FXOS_WriteReg(fxos_handle, CTRL_REG2, SLPE_MASK | SMOD_LOW_POWER | MOD_HIGH_RES) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//#else
//	/* enable auto-sleep, low power in sleep, high res in wake */
//	if(FXOS_WriteReg(fxos_handle, CTRL_REG2, MOD_HIGH_RES) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//
//#endif

	/* set up Mag OSR and Hybrid mode using M_CTRL_REG1, use default for Acc */
//	if(FXOS_WriteReg(fxos_handle, M_CTRL_REG1, (M_RST_MASK | M_OSR_MASK | M_HMS_MASK)) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//
//	/* Enable hyrid mode auto increment using M_CTRL_REG2 */
//	if(FXOS_WriteReg(fxos_handle, M_CTRL_REG2, (M_HYB_AUTOINC_MASK)) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}

//#ifdef EN_FFMT
//	/* enable FFMT for motion detect for X and Y axes, latch enable */
//	if(FXOS_WriteReg(fxos_handle, FF_MT_CFG_REG, XEFE_MASK | YEFE_MASK | ELE_MASK | OAE_MASK) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//#endif

//#ifdef SET_THRESHOLD
//	/* set threshold to about 0.25g */
//	if(FXOS_WriteReg(fxos_handle, FT_MT_THS_REG, 0x04) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//#endif



//#ifdef SET_DEBOUNCE
//	/* set debounce to zero */
//	if(FXOS_WriteReg(fxos_handle, FF_MT_COUNT_REG, 0x00) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//#endif

//#ifdef EN_AUTO_SLEEP
//	/* set auto-sleep wait period to 5s (=5/0.64=~8) */
//	if(FXOS_WriteReg(fxos_handle, ASLP_COUNT_REG, 8) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//#endif
	/* default set to 4g mode */
//	if(FXOS_WriteReg(fxos_handle, XYZ_DATA_CFG_REG, FULL_SCALE_4G) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}


//#ifdef EN_INTERRUPTS
//	/* enable data-ready, auto-sleep and motion detection interrupts */
//	/* FXOS1_WriteRegister(CTRL_REG4, INT_EN_DRDY_MASK | INT_EN_ASLP_MASK | INT_EN_FF_MT_MASK); */
//	if(FXOS_WriteReg(fxos_handle, CTRL_REG4, 0x0) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//	/* route data-ready interrupts to INT1, others INT2 (default) */
//	if(FXOS_WriteReg(fxos_handle, CTRL_REG5, INT_CFG_DRDY_MASK) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//	/* enable ffmt as a wake-up source */
//	if(FXOS_WriteReg(fxos_handle, CTRL_REG3, WAKE_FF_MT_MASK) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//	/* finally activate accel_device with ASLP ODR=0.8Hz, ODR=100Hz, FSR=2g */
//	if(FXOS_WriteReg(fxos_handle, CTRL_REG1, HYB_ASLP_RATE_0_8HZ | HYB_DATA_RATE_100HZ | ACTIVE_MASK) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//#else
//	/* Setup the ODR for 50 Hz and activate the accelerometer */
//	if(FXOS_WriteReg(fxos_handle, CTRL_REG1, (HYB_DATA_RATE_200HZ | ACTIVE_MASK)) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//#endif

	/* Read Control register again to ensure we are in active mode */
//	if(FXOS_ReadReg(fxos_handle, CTRL_REG1, tmp, 1) != kStatus_Success)
//	{
//		return kStatus_Fail;
//	}
//
//	if ((tmp[0] & ACTIVE_MASK) != ACTIVE_MASK)
//	{
//		return kStatus_Fail;
//	}



	static nrf_twi_mngr_transfer_t const fxos_init_transfers[] =
	{
		/* Put in standby */
		FXOS_STANDBY(tmp),
		/* Disable the FIFO */
		I2C_WRITE_REG(FXOS_7BIT_ADDRESS, F_SETUP_REG, F_MODE_DISABLED, 1),
#ifdef LPSLEEP_HIRES
		/* enable auto-sleep, low power in sleep, high res in wake */
		I2C_WRITE_REG(FXOS_7BIT_ADDRESS, CTRL_REG2, SLPE_MASK | SMOD_LOW_POWER | MOD_HIGH_RES, 1),
#else
		I2C_WRITE_REG(FXOS_7BIT_ADDRESS, CTRL_REG2, MOD_HIGH_RES, 1),
#endif
		/* set up Mag OSR and Hybrid mode using M_CTRL_REG1, use default for Acc */
		I2C_WRITE_REG(FXOS_7BIT_ADDRESS, M_CTRL_REG1, (M_RST_MASK | M_OSR_MASK | M_HMS_MASK), 1),
		/* Enable hyrid mode auto increment using M_CTRL_REG2 */
		I2C_WRITE_REG(FXOS_7BIT_ADDRESS, M_CTRL_REG2, (M_HYB_AUTOINC_MASK), 1),

#ifdef EN_FFMT
		/* enable FFMT for motion detect for X and Y axes, latch enable */
		I2C_WRITE_REG(FXOS_7BIT_ADDRESS, FF_MT_CFG_REG, (XEFE_MASK | YEFE_MASK | ELE_MASK | OAE_MASK), 1),
#endif

#ifdef SET_THRESHOLD
		/* set threshold to about 0.25g */
		I2C_WRITE_REG(FXOS_7BIT_ADDRESS, FT_MT_THS_REG, 0x04, 1),
#endif

#ifdef SET_DEBOUNCE
		/* set debounce to zero */
		I2C_WRITE_REG(FXOS_7BIT_ADDRESS, FF_MT_COUNT_REG, 0x00, 1),
#endif

#ifdef EN_AUTO_SLEEP
		/* set auto-sleep wait period to 5s (=5/0.64=~8) */
		I2C_WRITE_REG(FXOS_7BIT_ADDRESS, ASLP_COUNT_REG, 8, 1),
#endif
		/* default set to 4g mode */
		I2C_WRITE_REG(FXOS_7BIT_ADDRESS, XYZ_DATA_CFG_REG, FULL_SCALE_4G, 1),

		/* Setup the ODR for 50 Hz and activate the accelerometer */
		I2C_WRITE_REG(FXOS_7BIT_ADDRESS, CTRL_REG1, (HYB_DATA_RATE_200HZ | ACTIVE_MASK), 1),
	};

	i2c_perform(NULL, fxos_init_transfers, sizeof(fxos_init_transfers) / sizeof(fxos_init_transfers[0]), NULL);

	return;
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
		/* Get sensor range */
		g_sensorRange = g_fxosHandle->acc_range[0];

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

		// TODO Magnetometer_Calibrate(g_fxosHandle);
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

	LOG_INFO("Compass Angle: %d\r\n", (int)g_Yaw);

}
