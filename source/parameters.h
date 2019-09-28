/*
 * parameters.h
 *
 *  Created on: 11 nov. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_PARAMETERS_H_
#define SOURCE_PARAMETERS_H_

#define APP_TIMEOUT_DELAY_MS           5

#define S_TO_MS(X)                     ((X)*1000)

#define LS027_TIMEOUT_DELAY_MS         1250

#define GPS_USE_COLD_START             0

#define HISTO_POINT_SIZE               20

#define LNS_OVER_GPS_DTIME_S           6

#define SENSORS_READING_DELAY_MS       250

#define BOUCLE_FEC_UPDATE_RATE_MS      1000

#define BACKLIGHT_AUTO_START_RAW_VIS   5

#define STC3100_CUR_SENS_RES_MO        100

#define FXOS_MEAS_CAL_LIM_MS           60000

#define FXOS_MAG_FILTER_COEFF          0.5f

#define SEG_OFF_NB_POINTS              30

#define VH_RATIO                       0.8f

#define NB_SEG_ON_DISPLAY              2

#define LOCATOR_MAX_DATA_AGE_MS        4000

#define MIN_TIME_COMP_STOP_MS          5000

#define FEC_PW_BUFFER_NB_ELEM          (240/2)

#define BOUCLE_DEFAULT_MODE            eBoucleGlobalModesCRS
#define VUE_DEFAULT_MODE               eVueGlobalScreenCRS

#define USB_TIMEOUT_MS                 20

#define TOT_HEAP_MEM_AVAILABLE         __HEAP_SIZE

#define BARO_REFRESH_PER_MS            333

#define SENSORS_REFRESH_PER_MS         1000

#define ATT_BUFFER_NB_ELEM             5

#define FILTRE_NB                      15

#define USER_WEIGHT                    79U

#define USER_FTP                       256U

///// CODE FLAGS

#ifndef TDD
#define FRAM_PRESENT
#endif

#define BARO_TYPE                      bme280

//#define _DEBUG_TWI


#endif /* SOURCE_PARAMETERS_H_ */
