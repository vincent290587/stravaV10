/*
 * file_parser.h
 *
 *  Created on: 18 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_PARSERS_FILE_PARSER_H_
#define SOURCE_PARSERS_FILE_PARSER_H_

#include "Segment.h"
#include "Parcours.h"


int chargerPointSeg(char *buffer, Segment& mon_segment, float& time_start);

int chargerPointPar(char *buffer, Parcours& mon_parcours);

int parseSegmentName(const char *name, float *lat_, float *lon_);


#endif /* SOURCE_PARSERS_FILE_PARSER_H_ */
