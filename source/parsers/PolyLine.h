/*
 * PolyLine.h
 *
 *  Created on: 22 mai 2020
 *      Author: vgol
 */

#ifndef POLYLINE_H_
#define POLYLINE_H_

#include <stdint.h>
#include <vector>

#include "Base64.h"
#include "segger_wrapper.h"


class LatLng {
public:
	LatLng() {
		lat=0;
		lon=0;
	}
	LatLng(float _lat, float _lon) {
		lat=_lat;
		lon=_lon;
	}

	float lat;
	float lon;
};

class PolyLine {
public:
	PolyLine() {

	}

	int decodeBinaryPolyline(ByteBuffer& bArr) {

		_line.clear();

		int i;
		int i2;
		ByteBuffer bArr2 = bArr;
		int length = bArr2.length();
		int i3 = 8;
		int32_t b = (bArr2[0] & 255) | ((bArr2[1] & 255) << 8) | ((bArr2[2] & 255) << 16) | ((bArr2[3] & 255) << 24);
		uint8_t b2 = 7;
		int32_t i4 = (bArr2[4] & 255) | ((bArr2[5] & 255) << 8) | ((bArr2[6] & 255) << 16) | ((bArr2[7] & 255) << 24);
		_line.push_back(LatLng(((float) b) / 100000.0f, ((float) i4) / 100000.0f));
		while (i3 < length) {
			int i5 = 0;
			int i6 = 0;
			while (true) {
				i = i3 + 1;
				uint8_t b3 = bArr2[i3];
				i5 |= (b3 & 0xFF) << i6;
				i6 += b2;
				if (((b3 & 128) >> b2) == 0) {
					break;
				}
				i3 = i;
			}
			int32_t i7 = ((i5 & 1) != 0 ? ~(i5 >> 1) : i5 >> 1) + b;
			int i8 = 0;
			int i9 = 0;
			while (true) {
				i2 = i + 1;
				uint8_t b4 = bArr2[i];
				i8 |= (b4 & 0xFF) << i9;
				i9 += b2;
				if (((b4 & 128) >> b2) == 0) {
					break;
				}
				i = i2;
			}
			int i10 = i8 & 1;
			int i11 = i8 >> 1;
			if (i10 != 0) {
				i11 = ~i11;
			}
			i4 += i11;
			_line.push_back(LatLng(((float) i7) / 100000.0f, ((float) i4) / 100000.0f));
			b = i7;
			i3 = i2;
			b2 = 7;
		}

		return 0;
	}

	void toString(void) {

		NRF_LOG_RAW_INFO("PolyLine[%lu]: { \n", _line.size());
		for (uint16_t i=0; i< _line.size(); i++) {
			NRF_LOG_RAW_INFO("%f %f", _line[i].lat, _line[i].lon);
			if (i< _line.size()-1) {
				NRF_LOG_RAW_INFO(",\n");
			}
		}
		NRF_LOG_RAW_INFO("\n} \n");

	}

    std::vector<LatLng> _line;
};


#endif /* POLYLINE_H_ */
