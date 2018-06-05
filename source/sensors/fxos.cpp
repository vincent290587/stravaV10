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

#ifdef DEBUG_CONFIG
#define FXOS_7BIT_ADDRESS      0x1D
#else
#define FXOS_7BIT_ADDRESS      0x1E
#endif


status_t FXOS_ReadReg(fxos_handle_t *handle, uint8_t reg, uint8_t *val, uint8_t bytesNumber)
{
	status_t status = kStatus_Success;

	status = i2c0_read_reg(FXOS_7BIT_ADDRESS, reg, val, bytesNumber);

	return status;
}

status_t FXOS_WriteReg(fxos_handle_t *handle, uint8_t reg, uint8_t val)
{
	status_t status = kStatus_Success;
	uint8_t buff[1];

	buff[0] = val;

	status = i2c0_write_reg(FXOS_7BIT_ADDRESS, reg, buff, sizeof(buff));

	return status;
}

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define ACCEL_I2C_ADDR 1DU

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

	if (FXOS_ReadSensorData(g_fxosHandle, &fxos_data) != kStatus_Success)
	{
		LOG_INFO("Failed to read acceleration data!\r\n");
	}
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
		if (FXOS_ReadReg(g_fxosHandle, XYZ_DATA_CFG_REG, &g_sensorRange, 1) != kStatus_Success)
		{
			LOG_ERROR("FXOS Read fail\r\n");
			return;
		}
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

		Magnetometer_Calibrate(g_fxosHandle);
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
