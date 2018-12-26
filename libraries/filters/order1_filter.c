
#include "app_util_platform.h"
#include "order1_filter.h"


#include <string.h> // For memset

static const int lp_filter_numStages = 1;


void order1_filter_init( order1_filterType * pThis, float *p_filter_coefficients) {
	arm_biquad_cascade_df1_init_f32(&pThis->instance, lp_filter_numStages, p_filter_coefficients, pThis->state);
	order1_filter_reset( pThis );
}

void order1_filter_reset( order1_filterType * pThis ) {
	memset( &pThis->state, 0, sizeof( pThis->state ) ); // Reset state to 0
	pThis->output = 0;									// Reset output
}

int order1_filter_filterBlock( order1_filterType * pThis, float * pInput, float * pOutput, unsigned int count) {
	arm_biquad_cascade_df1_f32( &pThis->instance, pInput, pOutput, count );
	return count;
}

void order1_filter_writeInput( order1_filterType * pThis, float * pInput ) {
	arm_biquad_cascade_df1_f32( &pThis->instance, pInput, &pThis->output, 1 );
}

float order1_filter_readOutput( order1_filterType * pThis ) {
	return pThis->output;
}
