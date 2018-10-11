/*
 * EPONmeaPacket.cpp
 *
 *  Created on: 9 oct. 2018
 *      Author: Vincent
 */

#include <stdint.h>
#include "assert_wrapper.h"
#include "segger_wrapper.h"
#include "EPONmeaPacket.h"

int utc_to_gps_hour(int iYr, int iMo, int iDay, int iHr) {
	int iYearsElapsed; // Years since 1980
	int iDaysElapsed; // Days elapsed since Jan 6, 1980
	int iLeapDays; // Leap days since Jan 6, 1980
	int i;
	// Number of days into the year at the start of each month (ignoring leap years)
	const unsigned short doy[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243,
			273, 304, 334 };
	iYearsElapsed = iYr - 1980;
	i = 0;
	iLeapDays = 0;
	while (i <= iYearsElapsed) {
		if ((i % 100) == 20) {
			if ((i % 400) == 20) {
				iLeapDays++;
			}
		} else if ((i % 4) == 0) {
			iLeapDays++;
		}
		i++;
	}
	if ((iYearsElapsed % 100) == 20) {
		if (((iYearsElapsed % 400) == 20) && (iMo <= 2)) {
			iLeapDays--;
		}
	} else if (((iYearsElapsed % 4) == 0) && (iMo <= 2)) {
		iLeapDays--;
	}
	iDaysElapsed = iYearsElapsed * 365 + (int) doy[iMo - 1] + iDay + iLeapDays
			- 6;
	// Convert time to GPS weeks and seconds
	return (iDaysElapsed * 24 + iHr);
}

/**
 *
 * @param epo_data_
 * @param buffer_
 * @param max_size
 * @return
 */
uint16_t antenova_epo_packet(sEpoPacketSatData *epo_data_, uint8_t *buffer_, uint16_t max_size) {

	ASSERT(epo_data_);
	ASSERT(buffer_);

	uint16_t written = 0;

	memset(buffer_, 0, max_size);

	written += snprintf((char*)(buffer_ + written), max_size, "$PMTK721,%02X", epo_data_->sat_number);

	for (int i = 0; i < EPO_SAT_DATA_SIZE_BYTES; i++) {
		written += snprintf((char*)(buffer_ + written), max_size, "%02X", epo_data_->sat[i]);
	}

	uint8_t checksum = 0;

	for (int i = 1; i < written; i++) {
		checksum ^= buffer_[i];
	}

	written += snprintf((char*)(buffer_ + written), max_size, "*%02X\r\n", checksum);

	return written;
}



