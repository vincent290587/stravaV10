/*
 * BinnedData.h
 *
 *  Created on: 14 mrt. 2019
 *      Author: v.golle
 */

#ifndef SOURCE_MODEL_BINNEDDATA_H_
#define SOURCE_MODEL_BINNEDDATA_H_


#include <stdint.h>



class BinnedData {
public:
	BinnedData() {
	}

	virtual uint32_t getTimeMax(void)=0;
	virtual uint32_t getTimeTotal(void)=0;
	virtual uint32_t getTimeZX(uint16_t i)=0;
	virtual uint32_t getNbBins(void)=0;
	virtual uint32_t getCurBin(void)=0;

};


#endif /* SOURCE_MODEL_BINNEDDATA_H_ */
