/*
 * FXOS.h
 *
 *  Created on: 12 nov. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_SENSORS_FXOS_H_
#define SOURCE_SENSORS_FXOS_H_


#include "fsl_fxos.h"


#define FXOS_7BIT_ADDRESS      0x1E


/*=========================================================================*/

#define FXOS_STANDBY(p_buffer) \
		I2C_READ_REG_REP_START(FXOS_7BIT_ADDRESS, CTRL_REG1, p_buffer, 1),  \
		I2C_WRITE(FXOS_7BIT_ADDRESS, p_buffer[0] & (uint8_t)~ACTIVE_MASK, 1)

#define FXOS_RESET(...) \
		I2C_WRITE(FXOS_7BIT_ADDRESS, CMD_RESET, 1)

#define FXOS_READ_ALL(p_mag_buffer, p_acc_buffer) \
		I2C_READ_REG_REP_START(FXOS_7BIT_ADDRESS, OUT_X_MSB_REG, p_acc_buffer, 6), \
		I2C_READ_REG_REP_START(FXOS_7BIT_ADDRESS, M_OUT_X_MSB_REG, p_mag_buffer, 6)


/*=========================================================================*/

void fxos_init(void);

void fxos_tasks(fxos_handle_t *g_fxosHandle);

#endif /* SOURCE_SENSORS_FXOS_H_ */
