/*
 * parameters.h
 *
 *  Created on: 11 nov. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_PARAMETERS_H_
#define SOURCE_PARAMETERS_H_

#define S_TO_MS(X)                     ((X)*1000)

#define GPS_USE_COLD_START             0

#define HISTO_POINT_SIZE               8

#define LNS_OVER_GPS_DTIME_S           6

#define SENSORS_READING_DELAY_MS       250

#define BOUCLE_FEC_UPDATE_RATE_MS      1000

#define BACKLIGHT_AUTO_START_RAW_VIS   5

#define BATT_INT_RES                   0.155

#define STC3100_CUR_SENS_RES_MO        100

#define SEG_OFF_NB_POINTS              30

#define VH_RATIO                       0.8

#define NB_SEG_ON_DISPLAY              2

#define LOCATOR_MAX_DATA_AGE_MS        4000

#define MIN_TIME_COMP_STOP_MS          5000

#define FEC_PW_BUFFER_NB_ELEM          (240/2)

#define BOUCLE_DEFAULT_MODE            eBoucleGlobalModesFEC

#define USB_TIMEOUT_MS                 20

#define TOT_HEAP_MEM_AVAILABLE         180000

#endif /* SOURCE_PARAMETERS_H_ */
