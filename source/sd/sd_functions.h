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

int epo_file_size(void);
int epo_file_read(sEpoPacketSatData* epo_data, uint16_t size_);
bool epo_file_start(int current_gps_hour);
int epo_file_stop(bool toBeDeleted);

bool log_file_start(void);
int log_file_read(uint8_t* log_buffer, uint16_t size_);
int log_file_stop(bool toBeDeleted);

#if defined(__cplusplus)
}
#endif /* _cplusplus */

#endif /* SOURCE_SD_SD_FUNCTIONS_H_ */
