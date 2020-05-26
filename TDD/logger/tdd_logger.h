/*
 * tdd_logger.h
 *
 *  Created on: 25 oct. 2019
 *      Author: vgol
 */

#ifndef TDD_LOGGER_H_
#define TDD_LOGGER_H_

#include <stdint.h>
#include <stddef.h>

enum {
	TDD_LOGGING_TIME,
	TDD_LOGGING_P2D,
	TDD_LOGGING_P3D,
	TDD_LOGGING_SEG_DIST,
	TDD_LOGGING_NB_SEG_ACT,
	TDD_LOGGING_HP,
	TDD_LOGGING_ALPHA,
	TDD_LOGGING_ALPHA0,
	TDD_LOGGING_SIM_SLOPE,
	TDD_LOGGING_EST_SLOPE,
	TDD_LOGGING_ALT_SIM,
	TDD_LOGGING_ALT_EST,
	TDD_LOGGING_TOT_CLIMB,
	TDD_LOGGING_CUR_POWER,
	TDD_LOGGING_CUR_SPEED,
	TDD_LOGGING_BARO_DIFF,
};

typedef enum {
	eTddLoggingDataInteger,
	eTddLoggingDataSignedInteger,
	eTddLoggingDataFloat,
	eTddLoggingDataDouble,
} eTddLoggingDataType;

typedef struct {
	union {
		uint32_t data_int;
		int32_t data_sint;
		float data_float;
	};
	eTddLoggingDataType type;
	char name[50];
} sTddLoggingAtom;

typedef struct {
	int is_init;
	int is_started;
	size_t nb_atoms;
	sTddLoggingAtom atom_array[250];
} sTddLoggingMolecule;

#ifdef	__cplusplus
extern "C" {
#endif

void tdd_logger_init(const char * filname);

void tdd_logger_start(void);

void tdd_logger_log_name(size_t index, const char * name);

void tdd_logger_log_int(size_t index, uint32_t to_log);

void tdd_logger_log_sint(size_t index, int32_t to_log);

void tdd_logger_log_float(size_t index, float to_log);

void tdd_logger_flush(void);

#ifdef	__cplusplus
}
#endif

#endif /* TDD_LOGGER_H_ */
