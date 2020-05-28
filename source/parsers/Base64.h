#ifndef _MACARON_BASE64_H_
#define _MACARON_BASE64_H_

/**
 * The MIT License (MIT)
 * Copyright (c) 2016 tomykaira
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <string>
#include <vector>

const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline uint32_t b64_encoded_size(uint32_t inlen)
{
	uint32_t ret;

	// we want the upper multiple of 4
	if (inlen & 0b11) {
		inlen = ((inlen | 0b100) >> 2) << 2;
	}

	ret = inlen;
	if (inlen % 3 != 0)
		ret += 3 - (inlen % 3);
	ret /= 3;
	ret *= 4;

	return ret;
}

static inline char *b64_encode(const unsigned char *in, size_t len)
{
	char   *out;
	size_t  elen;
	size_t  i;
	size_t  j;
	size_t  v;

	if (in == NULL || len == 0)
		return NULL;

	elen = b64_encoded_size(len);
	out  = (char*)malloc(elen+1);
	out[elen] = '\0';

	for (i=0, j=0; i<len; i+=3, j+=4) {
		v = in[i];
		v = i+1 < len ? v << 8 | in[i+1] : v << 8;
		v = i+2 < len ? v << 8 | in[i+2] : v << 8;

		out[j]   = b64chars[(v >> 18) & 0x3F];
		out[j+1] = b64chars[(v >> 12) & 0x3F];
		if (i+1 < len) {
			out[j+2] = b64chars[(v >> 6) & 0x3F];
		} else {
			out[j+2] = '=';
		}
		if (i+2 < len) {
			out[j+3] = b64chars[v & 0x3F];
		} else {
			out[j+3] = '=';
		}
	}

	return out;
}

static const int b64invs[] = { 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58,
	59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
	21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51 };

static inline uint32_t b64_decoded_size(const char *in)
{
	uint32_t len;
	uint32_t ret;
	uint32_t i;

	if (in == NULL)
		return 0;

	len = strlen(in);
	ret = len / 4 * 3;

	for (i=len; i-->0; ) {
		if (in[i] == '=') {
			ret--;
		} else {
			break;
		}
	}

	// we want the upper multiple of 4
	if (ret & 0b11) {
		ret = ((ret | 0b100) >> 2) << 2;
	}

	return ret;
}

static inline int b64_isvalidchar(char c)
{
	if (c >= '0' && c <= '9')
		return 1;
	if (c >= 'A' && c <= 'Z')
		return 1;
	if (c >= 'a' && c <= 'z')
		return 1;
	if (c == '+' || c == '/' || c == '=')
		return 1;
	return 0;
}

static inline int b64_decode(const char *in, unsigned char *out, size_t outlen)
{
	size_t len;
	size_t i;
	size_t j;
	int    v;

	if (in == NULL || out == NULL)
		return 0;

	len = strlen(in);
	if (outlen < b64_decoded_size(in) || outlen % 4 != 0)
		return 0;

	for (i=0; i<len; i++) {
		if (!b64_isvalidchar(in[i])) {
			return 0;
		}
	}

	for (i=0, j=0; i<len; i+=4, j+=3) {
		v = b64invs[in[i]-43];
		v = (v << 6) | b64invs[in[i+1]-43];
		v = in[i+2]=='=' ? v << 6 : (v << 6) | b64invs[in[i+2]-43];
		v = in[i+3]=='=' ? v << 6 : (v << 6) | b64invs[in[i+3]-43];

		out[j] = (v >> 16) & 0xFF;
		if (in[i+2] != '=')
			out[j+1] = (v >> 8) & 0xFF;
		if (in[i+3] != '=')
			out[j+2] = v & 0xFF;
	}

	return 1;
}

class BaseString {
public:
	BaseString() {

	}

	void append(const char * buffer, size_t length) {

		for (size_t i=0; i< length; i++) {
			original.push_back((char)buffer[i]);
		}
	}

	const char * toString() {
		return original.data();
	}

	size_t length(void) {
		return original.size();
	}
private:
    std::vector<char> original;
};

class ByteBuffer {
public:
	ByteBuffer() {

	};

	void add(const unsigned char buffer[], size_t length) {

		for (size_t i=0; i< length; i++) {
			original.push_back((uint8_t)buffer[i]);
		}
	}

	void addU(const uint8_t buffer[], size_t length) {

		for (size_t i=0; i< length; i++) {
			original.push_back(buffer[i]);
		}
	}

	bool canGetInt(size_t pos) {

		if (pos+4 > length()) {
			return false;
		}

		return true;
	}

	uint32_t getInt(size_t pos, bool &is_error) {

		if (pos+4 > length()) {
			is_error = true;
			return 0;
		}

		uint32_t res = original[pos];
		res |= original[pos+1] << 8;
		res |= original[pos+2] << 16;
		res |= original[pos+3] << 24;

		//printf("getInt %lu : %ld \n", pos, (int32_t)res);

		return res;
	}

	size_t length(void) {
		return original.size();
	}

	uint8_t operator [](uint8_t pos) {
		return original[pos];
	}

	void toString(void) {

		printf("ByteBuffer: {");
		for (uint16_t i=0; i< original.size(); i++) {
			printf("0x%02X", original[i]);
			if (i< original.size()-1) {
				printf(",");
			}
		}
		printf("} \n");
	}

	const uint8_t * pData() {
		return original.data();
	}

    std::vector<uint8_t> original;
};

class Base64 {
public:
	Base64() {

	}

	ByteBuffer& decode(BaseString &in, uint8_t flags) {

		size_t         out_len;
		unsigned char  buffer[500];

		out_len = b64_decoded_size(in.toString());

		(void)b64_decode(in.toString(), buffer, out_len);

		_buffer.original.clear();
		_buffer.add(buffer, out_len);

		return _buffer;
	}

	void encode(ByteBuffer &in, BaseString &out) {

		char *p_str = b64_encode((const unsigned char*)in.pData(), in.length());

		out.append(p_str, b64_encoded_size(in.length()));
	}

private:
	ByteBuffer _buffer;
};

#endif /* _MACARON_BASE64_H_ */
