
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include "utils.h"

#include "fit_crc.h"
#include "PolyLine.h"
#include "ff.h"
#include "WString.h"
#include "sd_functions.h"
#include "inseg_handler.h"
#include "SequenceUnpacker.h"

#include "segger_wrapper.h"


class UploadSegment {
public:
	UploadSegment(uint8_t const id[], const float lat, const float lon) : startPnt(lat, lon) {

		memcpy(seg_id, id, 8);
		cur_size   = 0;
		total_size = 0;
		buffer     = nullptr;
	}
	~UploadSegment() {
		deallocate();
	}

	int isID(uint8_t const id[]) {
		return memcmp(seg_id, id, 8) == 0;
	}

	void copyID(uint8_t id[]) {
		memcpy(id, seg_id, 8);
	}

	void allocate(const uint32_t size) {
		if (!total_size) {
			total_size = size+20;
			buffer = new uint8_t[total_size];
		}

	}

	void deallocate() {
		if (buffer) {
			delete[] buffer;
			total_size = 0;
		}
	}

	int append(uint8_t const p_data[], uint32_t length) {

		if (buffer && length + cur_size <= total_size) {
			memcpy(buffer+cur_size, p_data, length);
			cur_size += length;
			return 0;
		}
		return -1;
	}

	uint8_t  seg_id[8];
	uint32_t cur_size;
	uint32_t total_size;
	uint8_t  *buffer;
	LatLng   startPnt;
};


static uint8_t  m_is_downloading = 0;
static uint32_t m_next_seg_idx;
static uint32_t m_cur_seg_idx;
static std::vector<UploadSegment> m_list_segs;

#define MAX_ARRAY_PRINTF        64

static void _dump_as_char(uint8_t const *p_data, uint16_t length) {

	if (!p_data) return;

	NRF_LOG_RAW_INFO("{");
	for (uint16_t i=0; i< length && i < MAX_ARRAY_PRINTF; i++) {
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


void sd_save_seg(UploadSegment &cur_u_seg) {

	static uint16_t nb = 0;

	char fname[20];
	snprintf(fname, sizeof(fname), "SEG_%04X.TXT", nb++);
	fname[12] = 0;

	if (!cur_u_seg.total_size) {
		NRF_LOG_ERROR("sd_save_seg empty file");
	}

	FIL g_fileObject;
	FRESULT error = f_open(&g_fileObject, fname, FA_WRITE | FA_CREATE_ALWAYS);
	APP_ERROR_CHECK(error);
	if (error)
	{
		NRF_LOG_ERROR("sd_save_seg open file failed.");
		return;
	}

	f_write(&g_fileObject, cur_u_seg.buffer, cur_u_seg.total_size, NULL);

	error = f_close(&g_fileObject);
	APP_ERROR_CHECK(error);
	if (error)
	{
		NRF_LOG_ERROR("sd_save_seg close file failed.");
		return;
	}

	NRF_LOG_INFO("sd_save_seg Saved as %s", fname);

}

void _handler_segment_parse(UploadSegment &cur_u_seg) {

	uint8_t index = 0;
	uint16_t nb_points_seg = 0;
	PolyLine myPoly;

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

	if (cur_u_seg.total_size == 0 ||
			!cur_u_seg.buffer) {
		NRF_LOG_ERROR("Empty segment");
		return;
	}


	task_delay(10);

	uint32_t dist = _decode_uint32_little(cur_u_seg.buffer+index);
	index+=4;
	NRF_LOG_INFO("dist : %lu", dist);

	float f_lat = FROM_SEMICIRCLES(_decode_uint32_little(cur_u_seg.buffer+index));
	index+=4;
	NRF_LOG_INFO("i_lat: %ld", (int32_t)f_lat);

	float f_lon = FROM_SEMICIRCLES(_decode_uint32_little(cur_u_seg.buffer+index));
	index+=4;
	NRF_LOG_INFO("i_lon: %ld", (int32_t)f_lon);

	int32_t i_lat = (int32_t)_decode_uint32_little(cur_u_seg.buffer+index) / 119;
	index+=4;
	NRF_LOG_INFO("i_lat: %ld", i_lat);

	int32_t i_lon = (int32_t)_decode_uint32_little(cur_u_seg.buffer+index) / 119;
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

	NRF_LOG_FLUSH();

	_dump_as_char(cur_u_seg.buffer+index, name_length); // name
	index+=name_length;

	uint16_t poly_length = _decode_uint16_little(cur_u_seg.buffer+index);
	index+=2;

	NRF_LOG_INFO("Polyline length: %u", poly_length);

	NRF_LOG_HEXDUMP_DEBUG(cur_u_seg.buffer+index, poly_length);
	{
		ByteBuffer bBuffer;
		bBuffer.addU(cur_u_seg.buffer+index, poly_length);

		myPoly.decodeBinaryPolyline(bBuffer);
		nb_points_seg = myPoly._line.size();
		//myPoly.toString();
	}
	index+=poly_length;

	NRF_LOG_FLUSH();

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

	// TODO save it as a blob
	sd_save_seg(cur_u_seg);
	return;

	NRF_LOG_HEXDUMP_DEBUG(cur_u_seg.buffer+index, comp_dist_length);
	{
		ByteBuffer bBuffer;
		bBuffer.addU(cur_u_seg.buffer+index, comp_dist_length);
		SequenceUnpacker unpacker(bBuffer, nb_points_seg-1);
		unpacker.unpack();

	}
	index+=comp_dist_length;

	uint16_t comp_stream_length = _decode_uint16_little(cur_u_seg.buffer+index);
	index+=2;

	NRF_LOG_INFO("Effort stream length: %u", comp_stream_length);

	NRF_LOG_HEXDUMP_DEBUG(cur_u_seg.buffer+index, comp_stream_length);
	//_dump_as_char(cur_u_seg.buffer+index, comp_stream_length);

	ByteBuffer bBuffer;
	bBuffer.addU(cur_u_seg.buffer+index, comp_stream_length);
	SequenceUnpacker effort_unpacker(bBuffer, nb_points_seg-1);
	effort_unpacker.unpack();
//	for (size_t i=0; i < effort_unpacker.original.size(); i++) {
//		NRF_LOG_INFO("Dt:  %ld", (int32_t)effort_unpacker.original[i]);
//	}

	index+=comp_stream_length;

	NRF_LOG_FLUSH();

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
	NRF_LOG_FLUSH();

	if (crc16 == _decode_uint16_little(cur_u_seg.buffer + index)) {

		// TODO save segment
		NRF_LOG_INFO("GOOD to save !");
	}

}

void inseg_handler_list_reset(void) {

	m_list_segs.clear();
	m_cur_seg_idx    = 0;
	m_next_seg_idx   = -1;
	m_is_downloading = 0;
}

void inseg_handler_list_input(uint8_t const seg_id[], float lat, float lon) {

	for (auto& curSeg : m_list_segs) {

		if (curSeg.isID(seg_id)) {
			return;
		}
	}

	m_list_segs.push_back(UploadSegment(seg_id, lat, lon));
}

void inseg_handler_list_process_start(void) {

	m_next_seg_idx   = 0;
	m_is_downloading = 0;
}

int inseg_handler_list_process_tasks(uint8_t seg_id[]) {

	if (m_list_segs.size() == 0) {
		return -1;
	}

	if (m_is_downloading) {
		return -2;
	}

	if (m_next_seg_idx < m_list_segs.size()) {

		// copy ID
		m_list_segs[m_next_seg_idx].copyID(seg_id);

		m_is_downloading = 1;

		m_next_seg_idx++;

		return 0;
	} else {
		// we are done
		return 1;
	}

	// unexpected
	return -3;
}

void inseg_handler_segment_start(uint8_t const seg_id[], uint32_t size) {

	if (size > 20000) {
		NRF_LOG_ERROR("inseg_handler_segment_start: segment size too gret (%lu) !!", size);
		return;
	}

	m_cur_seg_idx = 0;

	for (auto& curSeg : m_list_segs) {

		if (curSeg.isID(seg_id)) {

			curSeg.allocate(size);
			return;
		}

		m_cur_seg_idx++;
	}

	NRF_LOG_ERROR("inseg_handler_segment_start: segment not found !!");
}

void inseg_handler_segment_data(uint8_t is_end, uint8_t const p_data[], uint32_t length) {

	if (m_is_downloading == 0) {
		NRF_LOG_ERROR("inseg_handler_segment_data wrong state");
		return;
	}

	// get current segment
	UploadSegment &cur_u_seg = m_list_segs[m_cur_seg_idx];

	// copy segment data
	int ret = cur_u_seg.append(p_data, length);
	if (ret) {
		NRF_LOG_ERROR("inseg_handler_segment_data: append failed");
	} else {
		NRF_LOG_INFO("inseg_handler_segment_data: append %lu", cur_u_seg.cur_size);
	}

	NRF_LOG_FLUSH();

	if (is_end) {
		_handler_segment_parse(cur_u_seg);

		cur_u_seg.deallocate();

		m_is_downloading = 0;
	}
}
