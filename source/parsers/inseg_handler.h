/*
 * inseg_handler.h
 *
 *  Created on: 28 mai 2020
 *      Author: vgol
 */

#ifndef SOURCE_PARSERS_INSEG_HANDLER_H_
#define SOURCE_PARSERS_INSEG_HANDLER_H_

#include <stdint.h>


#define FROM_SEMICIRCLES(X)	             (((float)((int32_t)X) / 119.30464f))


#ifdef	__cplusplus
extern "C" {
#endif

void inseg_handler_list_reset(void);
void inseg_handler_list_input(uint8_t const seg_id[], float lat, float lon);
void inseg_handler_list_process_start(void);
int inseg_handler_list_process_tasks(uint8_t seg_id[]);

void inseg_handler_segment_start(uint8_t const seg_id[], uint32_t size);
void inseg_handler_segment_data(uint8_t is_end, uint8_t const p_data[], uint32_t length);

void inseg_handler_route_start(uint8_t const seg_id[], uint32_t size);
void inseg_handler_route_data(uint8_t is_end, uint8_t const p_data[], uint32_t length);

#ifdef	__cplusplus
}
#endif

#endif /* SOURCE_PARSERS_INSEG_HANDLER_H_ */
