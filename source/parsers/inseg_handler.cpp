
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include "utils.h"

#include "PolyLine.h"
#include "inseg_handler.h"
#include "SequenceUnpacker.h"

#include "segger_wrapper.h"

typedef struct {
	uint32_t cur_size;
	uint32_t total_size;
	uint64_t seg_id;
	uint8_t  *buffer;
} sUploadSegment;

class UploadSegment {
public:
	UploadSegment(uint8_t const id[], const float lat, const float lon) : startPnt(lat, lon) {

		memcpy(seg_id, id, 8);
		cur_size = 0;
		total_size = 0;
		buffer   = nullptr;
	}
	~UploadSegment() {
		deallocate();
	}

	int isID(uint8_t const id[]) {
		return memcmp(seg_id, id, 8) == 0;
	}

	void allocate(const uint32_t size) {
		total_size = size+20;
		if (total_size) buffer = new uint8_t[total_size];

	}

	void deallocate() {
		if (buffer) delete[] buffer;
	}

	uint8_t seg_id[8];
	uint32_t cur_size;
	uint32_t total_size;
	uint8_t  *buffer;
	LatLng   startPnt;
};


static uint32_t m_cur_seg_idx;
static std::vector<UploadSegment> m_list_segs;

#define MAX_ARRAY_PRINTF        64

static void _dump_as_char(uint8_t const *p_data, uint16_t length) {

	if (!p_data) return;

	NRF_LOG_RAW_INFO("{");
	for (uint16_t i=0; i< length && i < 32; i++) {
		NRF_LOG_RAW_INFO("%c", p_data[i]);
		if (i< length-1 && i < MAX_ARRAY_PRINTF-1) {
			NRF_LOG_RAW_INFO(",");
		}
	}
	NRF_LOG_RAW_INFO("} \n");
}

static uint32_t _decode_uint32_little(uint8_t const *p_data) {

	uint32_t res = 0;

	res = p_data[0];
	res |= p_data[1] << 8;
	res |= p_data[2] << 16;
	res |= p_data[3] << 24;

	return res;
}

static uint16_t _decode_uint16_little(uint8_t const *p_data) {

	uint16_t res = 0;

	res = p_data[0];
	res |= p_data[1] << 8;

	return res;
}

void _handler_segment_parse(UploadSegment &cur_u_seg) {

	uint8_t index = 0;
	uint16_t nb_points_seg = 0;

	// distance32
	// start_lat32
	// start_lon32
	// end_lat32
	// end_lon32
	// grade16
	// 00 00 00 00 00 00
	// name_length8
	// name[1..19]
	// poly_length16
	// poly[]
	// pr_time16
	// kom_time16
	// isHazardous16
	// tot_compressed_nb_words16
	// cStrDist_length16
	// compressedStreamDistance[]
	// cStrEffort_length16
	// compressedStreamEffort[]
	// padding_4_align
	// allocateSize16
	// crc16

	uint32_t dist = _decode_uint32_little(cur_u_seg.buffer+index);
	index+=4;
	NRF_LOG_INFO("dist : %lu", dist);

	int32_t i_lat = (int32_t)_decode_uint32_little(cur_u_seg.buffer+index) / 119;
	index+=4;
	NRF_LOG_INFO("i_lat: %ld", i_lat);

	int32_t i_lon = (int32_t)_decode_uint32_little(cur_u_seg.buffer+index) / 119;
	index+=4;
	NRF_LOG_INFO("i_lon: %ld", i_lon);

	i_lat = (int32_t)_decode_uint32_little(cur_u_seg.buffer+index) / 119;
	index+=4;
	NRF_LOG_INFO("i_lat: %ld", i_lat);

	i_lon = (int32_t)_decode_uint32_little(cur_u_seg.buffer+index) / 119;
	index+=4;
	NRF_LOG_INFO("i_lon: %ld", i_lon);

	int16_t grade = (int16_t)_decode_uint16_little(cur_u_seg.buffer+index);
	index+=2;
	NRF_LOG_INFO("grade * 10: %d", grade * 10 / 255);

	index+=6; // padding 0

	// dump name
	uint16_t name_length = cur_u_seg.buffer[index];
	index+=1;

	NRF_LOG_INFO("name_length: %u", name_length);

	_dump_as_char(cur_u_seg.buffer+index, name_length); // name
	index+=name_length;

	uint16_t poly_length = _decode_uint16_little(cur_u_seg.buffer+index);
	index+=2;

	NRF_LOG_INFO("Polyline length: %u", poly_length);

	NRF_LOG_HEXDUMP_INFO(cur_u_seg.buffer+index, poly_length);
	{
		ByteBuffer bBuffer;
		bBuffer.addU(cur_u_seg.buffer+index, poly_length);

		PolyLine myPoly;
		myPoly.decodeBinaryPolyline(bBuffer);
		nb_points_seg = myPoly._line.size();
		myPoly.toString();
	}
	index+=poly_length;

	// pr time
	uint16_t pr_time =  _decode_uint16_little(cur_u_seg.buffer+index);
	index+=2;

	// kom time
	uint16_t kom_time =  _decode_uint16_little(cur_u_seg.buffer+index);
	index+=2;

	NRF_LOG_INFO("PR/KOM times: %u %u", pr_time, kom_time);

	index+=2; // hazardous

	uint16_t words_comp =  _decode_uint16_little(cur_u_seg.buffer+index);
	index+=2;

	NRF_LOG_INFO("words_sum: %u", words_comp);

	uint16_t comp_dist_length = _decode_uint16_little(cur_u_seg.buffer+index);
	index+=2;

	NRF_LOG_INFO("Dist stream length: %u", comp_dist_length);

	NRF_LOG_HEXDUMP_INFO(cur_u_seg.buffer+index, comp_dist_length);
	{
		ByteBuffer bBuffer;
		bBuffer.addU(cur_u_seg.buffer+index, comp_dist_length);
		SequenceUnpacker unpacker(bBuffer, nb_points_seg-1);
		unpacker.unpack();
		for (size_t i=0; i < unpacker.original.size(); i++) {
			printf("Ddist:  %ld \n", (int32_t)unpacker.original[i]);
		}
	}
	index+=comp_dist_length;

	uint16_t comp_stream_length = _decode_uint16_little(cur_u_seg.buffer+index);
	index+=2;

	NRF_LOG_INFO("Effort stream length: %u", comp_stream_length);

	NRF_LOG_HEXDUMP_INFO(cur_u_seg.buffer+index, comp_stream_length);
	//_dump_as_char(cur_u_seg.buffer+index, comp_stream_length);
	{
		ByteBuffer bBuffer;
		bBuffer.addU(cur_u_seg.buffer+index, comp_stream_length);
		SequenceUnpacker unpacker(bBuffer, nb_points_seg-1);
		unpacker.unpack();
		for (size_t i=0; i < unpacker.original.size(); i++) {
			printf("Dt:  %ld \n", (int32_t)unpacker.original[i]);
		}
	}

	index+=comp_stream_length;

	uint16_t nb_padding = ((words_comp-1) << 2) - (comp_dist_length + comp_stream_length);
	NRF_LOG_INFO("Nb padding: %u", nb_padding);
	index+= nb_padding;

	uint16_t allocate_length = _decode_uint16_little(cur_u_seg.buffer+index);
	index+=2;
	NRF_LOG_INFO("allocate_length: %u", allocate_length);

	uint16_t crc16 = 0;
    for (int i5 = 0; i5 < index; i5++) {
    	crc16 = FitCRC_Get16(crc16, cur_u_seg.buffer[i5]);
    }
	NRF_LOG_INFO("CRC16 : %lu vs %lu", crc16, _decode_uint16_little(cur_u_seg.buffer + index));

}

void inseg_handler_list_reset(void) {

	m_list_segs.clear();
}

void inseg_handler_list_input(uint8_t const seg_id[], float lat, float lon) {

	for (auto& curSeg : m_list_segs) {

		if (curSeg.isID(seg_id)) {
			return;
		}
	}

	m_list_segs.push_back(UploadSegment(seg_id, lat, lon));
}

static uint8_t m_is_downloading = 0;

void inseg_handler_list_process_start(void) {

	m_cur_seg_idx    = 0;
	m_is_downloading = 0;
}

int inseg_handler_list_process_tasks(uint8_t seg_id[]) {

	if (m_list_segs.size() == 0) {
		return -1;
	}

	if (m_is_downloading) {
		return -2;
	}

	if (m_cur_seg_idx < m_list_segs.size()) {

		// TODO copy ID

		m_cur_seg_idx++;
		return 0;
	}

	// no more segments
	return 1;
}

void inseg_handler_segment_start(uint8_t const seg_id[], uint32_t size) {

	if (size > 12000) {
		NRF_LOG_ERROR("inseg_handler_list_input: segment size too gret (%lu) !!", size);
		return;
	}

	m_cur_seg_idx = 0;

	for (auto& curSeg : m_list_segs) {

		if (curSeg.isID(seg_id)) {

			m_is_downloading = 1;
			curSeg.allocate(size);
		}

		m_cur_seg_idx++;
	}

}

void inseg_handler_segment_data(uint8_t is_end, uint8_t const p_data[], uint32_t length) {

	// get current segment
	UploadSegment &cur_u_seg = m_list_segs[m_cur_seg_idx];

	// copy segment data
	memcpy(cur_u_seg.buffer+cur_u_seg.cur_size, p_data + 1, length);

	cur_u_seg.cur_size += length;

	if (is_end) {
		_handler_segment_parse(cur_u_seg);

		cur_u_seg.deallocate();
	}
}
