/*
 * tdd_logger.c
 *
 *  Created on: 11 dec. 2019
 *      Author: Vincent
 */

#include "tdd_logger.h"
#include "assert_wrapper.h"
#include <stdio.h>
#include <string.h>



static char filename[25];
static FILE *m_logging_file = NULL;
static sTddLoggingMolecule m_core;


void tdd_logger_init(const char * filname) {

	strncpy(filename, filname, sizeof(filename));

	memset(&m_core, 0, sizeof(sTddLoggingMolecule));

	m_logging_file = fopen(filename, "w+");
	fclose(m_logging_file);
	m_logging_file = 0;

}

void tdd_logger_start(void) {
	m_core.is_started = 1;
}

void tdd_logger_log_name(size_t index, const char * name) {

	strncpy(m_core.atom_array[index].name, name, sizeof(m_core.atom_array[index].name));
	m_core.nb_atoms++;
}

void tdd_logger_log_int(size_t index, uint32_t to_log) {

	if (!m_core.is_started) return;

	ASSERT(index < m_core.nb_atoms);

	m_core.atom_array[index].type = eTddLoggingDataInteger;
	m_core.atom_array[index].data_int = to_log;

}

void tdd_logger_log_sint(size_t index, int32_t to_log) {

	if (!m_core.is_started) return;

	ASSERT(index < m_core.nb_atoms);

	m_core.atom_array[index].type = eTddLoggingDataSignedInteger;
	m_core.atom_array[index].data_sint = to_log;

}

void tdd_logger_log_float(size_t index, float to_log) {

	if (!m_core.is_started) return;

	ASSERT(index < m_core.nb_atoms);

	m_core.atom_array[index].type = eTddLoggingDataFloat;
	m_core.atom_array[index].data_float = to_log;

}

void tdd_logger_flush(void) {

	if (!m_core.is_started) return;

	m_logging_file = fopen(filename, "a+");

	if (m_logging_file) {

		// print columns names
		if (!m_core.is_init) {
			for (uint16_t i=0; i < m_core.nb_atoms; i++) {

				char *term = i==m_core.nb_atoms-1 ? "\n":"\t";

				ASSERT(strlen(m_core.atom_array[i].name));

				fprintf(m_logging_file, "%s%s", m_core.atom_array[i].name, term);

			}

			m_core.is_init = 1;
		}

		// print data
		for (uint16_t i=0; i < m_core.nb_atoms; i++) {

			char *term = i==m_core.nb_atoms-1 ? "\n":"\t";

			switch (m_core.atom_array[i].type) {
			case eTddLoggingDataInteger:
			{
				fprintf(m_logging_file, "%u%s", m_core.atom_array[i].data_int, term);
			}
			break;
			case eTddLoggingDataSignedInteger:
			{
				fprintf(m_logging_file, "%d%s", m_core.atom_array[i].data_sint, term);
			}
			break;
			case eTddLoggingDataFloat:
			{
				fprintf(m_logging_file, "%.8E%s", m_core.atom_array[i].data_float, term);
			}
			break;
			break;
			default:
				break;
			}

		}

		fclose(m_logging_file);
		m_logging_file = 0;
	}
}
