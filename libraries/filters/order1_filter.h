/******************************* SOURCE LICENSE *********************************
Copyright (c) 2018 MicroModeler.

A non-exclusive, nontransferable, perpetual, royalty-free license is granted to the Licensee to
use the following Information for academic, non-profit, or government-sponsored research purposes.
Use of the following Information under this License is restricted to NON-COMMERCIAL PURPOSES ONLY.
Commercial use of the following Information requires a separately executed written license agreement.

This Information is distributed WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 ******************************* END OF LICENSE *********************************/

// A commercial license for MicroModeler DSP can be obtained at http://www.micromodeler.com/launch.jsp

// Begin header file, lp_filter.h

#ifndef LP_FILTER_H_ // Include guards
#define LP_FILTER_H_

#include "math_wrapper.h"

#ifndef TDD

typedef struct
{
	arm_biquad_casd_df1_inst_f32 instance;
	float32_t state[4];
	float32_t output;
	float lp_filter_coefficients[5];
} order1_filterType;
#else

typedef struct
{
	float state[4];
	float output;
	float lp_filter_coefficients[5];
} order1_filterType;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void order1_filter_init( order1_filterType * pThis, float *p_filter_coefficients);

void order1_filter_reset( order1_filterType * pThis );

void order1_filter_writeInput( order1_filterType * pThis, float * pInput );

float order1_filter_readOutput( order1_filterType * pThis );

#ifdef __cplusplus
}
#endif

#endif // LP_FILTER_H_

