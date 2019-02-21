
#include "order1_filter.h"

#include <stdlib.h> // For malloc/free
#include <string.h> // For memset


static const int lp_filter_numStages = 1;
static const int lp_filter_coefficientLength = 5;

typedef struct
{
	float *pInput;
	float *pOutput;
	float *pState;
	float *pCoefficients;
	short count;
} order1_filter_executionState;

static void order1_filter_filterBiquad( order1_filter_executionState * pExecState );


void order1_filter_init( order1_filterType * pThis, float *p_filter_coefficients)
{
	order1_filter_reset( pThis ); // Reset state to 0
	memcpy(pThis->lp_filter_coefficients, p_filter_coefficients, sizeof(pThis->lp_filter_coefficients));
}

void order1_filter_reset( order1_filterType * pThis )
{
	memset(&pThis->state, 0, sizeof( pThis->state )); // Reset state to 0
	pThis->output = 0;									// Reset output

}

int order1_filter_filterBlock( order1_filterType * pThis, float * pInput, float * pOutput, unsigned int count )
{
	order1_filter_executionState executionState;          // The executionState structure holds call data, minimizing stack reads and writes
	if( ! count ) return 0;                         // If there are no input samples, return immediately
	executionState.pInput = pInput;                 // Pointers to the input and output buffers that each call to filterBiquad() will use
	executionState.pOutput = pOutput;               // - pInput and pOutput can be equal, allowing reuse of the same memory.
	executionState.count = count;                   // The number of samples to be processed
	executionState.pState = pThis->state;                   // Pointer to the biquad's internal state and coefficients.
	executionState.pCoefficients = &pThis->lp_filter_coefficients[0];    // Each call to filterBiquad() will advance pState and pCoefficients to the next biquad

	// The 1st call to filter1_filterBiquad() reads from the caller supplied input buffer and writes to the output buffer.
	// The remaining calls to filterBiquad() recycle the same output buffer, so that multiple intermediate buffers are not required.

	order1_filter_filterBiquad( &executionState );		// Run biquad #0
	executionState.pInput = executionState.pOutput;         // The remaining biquads will now re-use the same output buffer.

	// At this point, the caller-supplied output buffer will contain the filtered samples and the input buffer will contain the unmodified input samples.
	return count;		// Return the number of samples processed, the same as the number of input samples

}

static void order1_filter_filterBiquad( order1_filter_executionState * pExecState )
{

	// Read state variables
	float x0;
	float x1 = pExecState->pState[0];
	float x2 = pExecState->pState[1];
	float y1 = pExecState->pState[2];
	float y2 = pExecState->pState[3];

	// Read coefficients into work registers
	float b0 = *(pExecState->pCoefficients++);
	float b1 = *(pExecState->pCoefficients++);
	float b2 = *(pExecState->pCoefficients++);
	float a1 = *(pExecState->pCoefficients++);
	float a2 = *(pExecState->pCoefficients++);

	// Read source and target pointers
	float *pInput  = pExecState->pInput;
	float *pOutput = pExecState->pOutput;
	short count = pExecState->count;
	float accumulator;

	while( count-- )
	{
		x0 = *(pInput++);

		accumulator  = x2 * b2;
		accumulator += x1 * b1;
		accumulator += x0 * b0;

		x2 = x1;		// Shuffle left history buffer
		x1 = x0;

		accumulator += y2 * a2;
		accumulator += y1 * a1;

		y2 = y1;		// Shuffle right history buffer
		y1 = accumulator ;

		*(pOutput++) = accumulator ;
	}

	*(pExecState->pState++) = x1;
	*(pExecState->pState++) = x2;
	*(pExecState->pState++) = y1;
	*(pExecState->pState++) = y2;


}

void order1_filter_writeInput(order1_filterType * pThis, float *input) {
	order1_filter_filterBlock( pThis, input, &(pThis)->output, 1);
}

float order1_filter_readOutput(order1_filterType * pThis) {
	return pThis->output;
}
