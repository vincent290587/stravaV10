

/*
 * XXX
 *
 *  Created on: 20 mai 2020
 *      Author: vgol
 */


#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "Model.h"
#include "millis.h"
#include "Attitude.h"
#include "sd_hal.h"
#include "segger_wrapper.h"

#include "ff.h"
#include "sd_functions.h"

#include "encode_lib.h"


static char fname[20];
static uint32_t m_nb_pos_saved = 0;
static uint32_t m_eof_ptr = 0;
static sEncodingData e_data;
static FIL g_fileObject;

extern "C" void UserData_GoTo(eUserDataPos pos)
{
	/**
	 * SEEK_END : It denotes end of the file.
	 * SEEK_SET : It denotes starting of the file.
	 */
	if (pos == eUserDataPosStart) {

		//fseek (fp , 0 , SEEK_SET);
		f_lseek (&g_fileObject , 0);

	} else if (pos == eUserDataPosEnd) {

		f_lseek (&g_fileObject , m_eof_ptr);
	}
}

extern "C" long UserData_Ftell(void)
{
	return m_eof_ptr;
}

extern "C" void WriteDataBare(const void *data, uint16_t data_size)
{
	f_write(&g_fileObject, data, data_size, NULL);
	m_eof_ptr += data_size;
}

void fit_terminate(void) {

	FRESULT error;

	if (!is_fat_init()) return;

	if (m_nb_pos_saved == 0) {
		return;
	}

	// open file
	error = f_open(&g_fileObject, fname, FA_WRITE);
	APP_ERROR_CHECK(error);
	if (error)
	{
		LOG_ERROR("Open FIT file %s failed !", fname);
		return;
	}

	UserData_GoTo(eUserDataPosEnd);

	// end record
	e_data.cmd = eEncodingCommandStop;
	encode_process(&e_data);

	// close file
	error = f_close(&g_fileObject);
	APP_ERROR_CHECK(error);
	if (error)
	{
		LOG_INFO("Close FIT file failed.");
		return;
	}

	memset(fname, 0, sizeof(fname));
	m_nb_pos_saved = 0;
	m_eof_ptr = 0;
}

/**
 *
 * @param att
 * @param nb_pos
 */
void fit_save_pos_buffer(SAttTime att[], uint16_t nb_pos) {

	BYTE mode = FA_OPEN_APPEND | FA_WRITE;

	if (!is_fat_init()) return;

	// calculate file name
	if (m_nb_pos_saved == 0 &&
			att[0].date.timestamp > 0) {

		memset(fname, 0, sizeof(fname));
		snprintf(fname, sizeof(fname), "%08lX.FIT", att[0].date.timestamp);

		// force file creation
		mode = FA_WRITE | FA_CREATE_NEW;

	} else if (att[0].date.timestamp == 0) {

		m_nb_pos_saved = 0;
		m_eof_ptr = 0;

		return;
	}

	// open file
	FRESULT error = f_open(&g_fileObject, fname, mode);
	APP_ERROR_CHECK(error);
	if (error)
	{
		LOG_ERROR("Open FIT file %s failed.", fname);
		return;
	}

	// prepare file headers
	if (m_nb_pos_saved == 0) {

		// init FIT file
		memset(&e_data, 0, sizeof(e_data));
		m_eof_ptr = 0;

		// start record
		e_data.timestamp = att[0].date.timestamp;
		e_data.cmd = eEncodingCommandStart;

		encode_process(&e_data);
	}

	int8_t cur_temp = (int8_t)stc.getTemperature();

	// save all buffers
	for (uint16_t i=0; i< nb_pos; i++) {

		memset(&e_data, 0, sizeof(e_data));

		uint32_t unix_timestamp = att[i].date.timestamp;

		// convert to ints
		int32_t lat=619451178, lon=53228942, alt = -230;

		lat = (int32_t)(att[i].loc.lat * 1.1930464E7f); // conversion to semicircles
		lon = (int32_t)(att[i].loc.lon * 1.1930464E7f);
		alt = (int32_t)(att[i].loc.alt * 1e2f);

		e_data.cmd = eEncodingCommandNone;
		e_data.lat = lat;
		e_data.lon = lon;
		e_data.alt_cm = alt;
		e_data.hrm = att[i].sensors.bpm;
		e_data.cad = att[i].sensors.cadence;
		e_data.temp_c = cur_temp;
		e_data.timestamp = unix_timestamp;

		encode_process(&e_data);

		m_nb_pos_saved++;

	}

	// close file
	error = f_close(&g_fileObject);
	APP_ERROR_CHECK(error);
	if (error)
	{
		LOG_ERROR("Close FIT file failed.");
		return;
	} else {
		LOG_INFO("Points added to FIT: %u", nb_pos);
	}

}

