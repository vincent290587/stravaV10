/*
 * VueCommon.h
 *
 *  Created on: 21 janv. 2018
 *      Author: Vincent
 */

#ifndef SOURCE_VUE_VUECOMMON_H_
#define SOURCE_VUE_VUECOMMON_H_

#define CLR_NRM                  1,0

typedef uint16_t tHistoValue;

typedef tHistoValue (*p_f_ringbuffer_read)(uint16_t);

typedef struct {
	int16_t nb_elem_tot;
	int16_t cur_elem_nb;
	tHistoValue max_value;
	tHistoValue ref_value;
	p_f_ringbuffer_read p_f_read;
} sVueHistoConfiguration;

#endif /* SOURCE_VUE_VUECOMMON_H_ */
