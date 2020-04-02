/*
 * sd_functions.h
 *
 *  Created on: 18 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_SD_SD_FUNCTIONS_H_
#define SOURCE_SD_SD_FUNCTIONS_H_

#include "Attitude.h"
#include "EpoDefinitions.h"

typedef enum {
	eSDTaskQueryNone,
	eSDTaskQueryHisto,
	eSDTaskQueryFile,
} eSDTaskQuery;

#if defined(__cplusplus)

class Segment;
class Parcours;

int load_segment(Segment& seg);

int load_parcours(Parcours& mon_parcours);

float segment_allocator(Segment& mon_seg, float lat1, float long1);

void sd_save_pos_buffer(SAttTime* att, uint16_t nb_pos);

extern "C" {
#endif /* _cplusplus */

int init_liste_segments(void);

void uninit_liste_segments(void);

int sd_functions__start_query(eSDTaskQuery query, const char * const fname);
int sd_functions__run_query(int restart, sCharArray *p_array, size_t max_size);
int sd_functions__stop_query(void);

uint16_t sd_functions__query_histo_list(int restart, sCharArray *p_array, size_t max_size);

int   sd_functions__query_file_start(const char * const fname);
char* sd_functions__query_file_run(sCharArray *p_array, size_t max_size);
int   sd_functions__query_file_stop(bool toBeDeleted);

int epo_file_size(void);
int epo_file_read(sEpoPacketSatData* sat_data, uint16_t size_);
bool epo_file_start(int current_gps_hour);
int epo_file_stop(bool toBeDeleted);

#if defined(__cplusplus)
}
#endif /* _cplusplus */

#endif /* SOURCE_SD_SD_FUNCTIONS_H_ */
