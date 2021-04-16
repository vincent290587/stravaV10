/*
 * SequenceUnpacker.h
 *
 *  Created on: 22 mai 2020
 *      Author: vgol
 */

#ifndef SEQUENCEUNPACKER_H_
#define SEQUENCEUNPACKER_H_

#include <stdint.h>
#include <vector>

#include "Base64.h"



class SequenceUnpacker {
public:
    SequenceUnpacker(BaseString& str, int i) {

    	ByteBuffer &wrap = myBase64.decode(str, 0);
    	this->decompressed = wrap;
    	this->decompressed.toString();

    	this->count = i;
    	this->numberOfBytes = wrap.length();

    }
    SequenceUnpacker(ByteBuffer& str, int i) {

    	this->decompressed = str;
    	this->decompressed.toString();

    	this->count = i;
    	this->numberOfBytes = str.length();

    }

    uint32_t getBits(int i) {
    	uint32_t i2 = this->bitCount;
    	uint32_t i3 = i2 & 31;
    	uint32_t i4 = i2 >> 5;
        if (i4 > (this->numberOfBytes >> 2)) {
            this->done = true;
        }
        this->bitCount += i;
    	//printf("bitCount %lu \n", this->bitCount);
        //try {
            bool is_error = false;
        	int32_t i5 = (int32_t)this->decompressed.getInt(i4 * 4, is_error) >> i3;
    		//printf("getInt %ld \n", (int32_t)this->decompressed.getInt(i4 * 4));
        	int32_t i6 = 32 - i3;
        	int32_t i7 = i - i6;
            if (i7 > 0) {
//                try {
            		if (is_error) {
            			this->done = true;
            		}
                	//printf("i50 %ld ", (int32_t)i5);
                    i5 = (i5 & ((uint32_t)-1 >> (32 - i6)));
                    //printf("  i51 %ld ", i5);
                    uint32_t tmp = this->decompressed.getInt((i4 + 1) * 4, is_error);
                    i5 |= (tmp & (((uint32_t)-1) >> (32 - i7))) << i6;
                    //printf("  i52 %ld \n", (((uint32_t)-1) >> (32 - i7)) << i6);
            		//printf("getInt %ld (%lu) \n", (int32_t)tmp, i4);
                    //printf("i5 %ld i6 %ld \n", i5, i6);
            		if (is_error) {
            			this->done = true;
            		}
//                } catch (IndexOutOfBoundsException unused) {
//                    this->done = true;
//                    return 0;
//                }
            }
            uint32_t i8 = 32 - i;
            return (i5 << i8) >> i8;
//        } catch (IndexOutOfBoundsException unused2) {
//            this->done = true;
//            return 0;
//        }
    }

//    L_0x0314:
//        int r8 = r8.length()
//        r1.append(r8)
//        java.lang.String r1 = r1.toString()
//        android.util.Log.i(r4, r1)
//        java.lang.String r1 = "komTimeSequence"
//        java.lang.String r8 = "distanceSequence"
//        if (r21 == 0) goto L_0x0385
//        r5 = 0
//        org.joda.time.DateTime r5 = (org.joda.time.DateTime) r5
//        r2.prDate = r5
//        int r5 = r7.size()
//        r6 = 2
//        if (r5 <= r6) goto L_0x035f
//        int r5 = r9.size()
//        if (r5 <= r6) goto L_0x035f
//        com.lezyne.gpsally.tools.SequencePacker r0 = new com.lezyne.gpsally.tools.SequencePacker
//        java.util.Collection r7 = (java.util.Collection) r7
//        int[] r1 = kotlin.collections.CollectionsKt.toIntArray(r7)
//        r0.<init>(r1)
//        java.lang.String r0 = r0.pack()
//        r2.compressedStreamDistance = r0
//        com.lezyne.gpsally.tools.SequencePacker r0 = new com.lezyne.gpsally.tools.SequencePacker
//        r13 = r9
//        java.util.Collection r13 = (java.util.Collection) r13
//        int[] r1 = kotlin.collections.CollectionsKt.toIntArray(r13)
//        r0.<init>(r1)
//        java.lang.String r0 = r0.pack()
//        r2.compressedKomStreamTime = r0
//        goto L_0x0401
//    L_0x035f:
//        com.lezyne.gpsally.tools.SequencePacker r5 = new com.lezyne.gpsally.tools.SequencePacker
//        kotlin.jvm.internal.Intrinsics.checkExpressionValueIsNotNull(r3, r8)
//        int[] r3 = r2.differenceCode((java.util.ArrayList<java.lang.Integer>) r3)
//        r5.<init>(r3)
//        java.lang.String r3 = r5.pack()
//        r2.compressedStreamDistance = r3
//        com.lezyne.gpsally.tools.SequencePacker r3 = new com.lezyne.gpsally.tools.SequencePacker
//        kotlin.jvm.internal.Intrinsics.checkExpressionValueIsNotNull(r0, r1)
//        int[] r0 = r2.differenceCode((java.util.ArrayList<java.lang.Integer>) r0)
//        r3.<init>(r0)
//        java.lang.String r0 = r3.pack()
//        r2.compressedKomStreamTime = r0
//        goto L_0x0401


    void unpack() {

    	//r7 = this;
    	uint32_t r0 = 32;
    	uint32_t r1 = this->getBits(r0);
    	this->original.push_back(r1);
    	//r2 = 3;
    	uint32_t r3 = this->getBits(3);
    	r3 = r3 & 7;
    	r3 = r3 + 2;
    	this->done = false;
    	while (!this->done) {
	    	//printf("bitCount %lu %lu \n", this->bitCount, this->numberOfBytes*8);
    		//L_0x0018:
    		uint32_t r4 = this->getBits(r3);
    		//printf();
    		int32_t r52 = -2147483648;
    		uint32_t r6 = 32 - r3;
    		uint32_t r5 = r52 >> r6;
    		if (r4 == r5) {
    			r3 = this->getBits(3);
    			r3 = r3 & 7;
    			r3 = r3 + 2;
    			r4 = this->getBits(r3);
    		}
    		//L_0x002f:
			r1 = r1 + r4;
//			std::vector<uint32_t> arr4 = this->original;
			//java.lang.Integer arr5 = java.lang.Integer.valueOf(-r1);
			this->original.push_back(r1);
//			std::vector<uint32_t> arr41 = this->original;
			uint32_t r41 = this->original.size();
			uint32_t r51 = this->count;
			if (r41 == r51) {
				break;//goto L_0x0044;
			}                //goto L_0x0048;
			//L_0x0044:
			//boolean r4 = this->done;
			if (this->bitCount >= this->numberOfBytes * 8) {
				break;
			}
    	}
    	//if (!this->done) goto L_0x0018;
    	//L_0x0048:
    	//    return;

    	//throw new UnsupportedOperationException("Method not decompiled: com.lezyne.gpsally.tools.SequenceUnpacker.unpack():void");
    }

    std::vector<uint32_t> original;

private:
    uint32_t bitCount = 0;
    uint32_t count = 0;
    ByteBuffer decompressed;
    bool done = false;
    uint32_t numberOfBytes;
    Base64 myBase64;
};



#endif /* SEQUENCEUNPACKER_H_ */
