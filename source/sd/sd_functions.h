/*
 * sd_functions.h
 *
 *  Created on: 18 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_SD_SD_FUNCTIONS_H_
#define SOURCE_SD_SD_FUNCTIONS_H_


#include "EpoDefinitions.h"

//#if defined(__cplusplus)
//extern "C" {
//#endif /* _cplusplus */

class Segment;
class Parcours;

int init_liste_segments(void);

int load_segment(Segment& seg);

int load_parcours(Parcours& mon_parcours);

float segment_allocator(Segment& mon_seg, float lat1, float long1);

int epo_file_size(void);
int epo_file_read(sEpoPacketSatData* epo_data);
int epo_file_start(void);
int epo_file_stop(bool toBeDeleted);


//#if defined(__cplusplus)
//}
//#endif /* _cplusplus */

#endif /* SOURCE_SD_SD_FUNCTIONS_H_ */
