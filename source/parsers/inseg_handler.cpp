
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include "utils.h"

#include "fit_crc.h"
#include "PolyLine.h"
#include "ff.h"
#include "sd_hal.h"
#include "WString.h"
#include "Model.h"
#include "ListePoints.h"
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
		poly_index = 0;
		effort_index = 0;
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
		if (!total_size && size > 0) {
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
	uint16_t poly_index;
	uint16_t effort_index;
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


void sd_save_seg(PolyLine& poly) {

	if (!is_fat_init()) return;

	// calculate name
	int32_t lat = (int32_t) ((poly._line[0].lat + 90) * 100000);
	int32_t lon = (int32_t) ((poly._line[1].lon + 180) * 100000);

	String lon36 = String(lon, 36);
	String f_name;
	f_name = String(lat, 36);
	f_name += '#';
	f_name += lon36.substring(0, 2);
	f_name += ".";
	f_name += lon36.substring(2, lon36.length());
	f_name = f_name.toUpperCase();

	String notif = "Saving file ";
	notif += f_name;

	model_add_notification("APP", notif.c_str(), 5, eNotificationTypeComplete);

	FIL g_fileObject;
	FRESULT error = f_open(&g_fileObject, f_name.c_str(), FA_WRITE | FA_CREATE_ALWAYS);
	APP_ERROR_CHECK(error);
	if (error)
	{
		NRF_LOG_ERROR("sd_save_seg open file failed.");
		return;
	}

	char buffer[200];
	for (uint16_t i=0; i < poly._line.size(); i++) {

		// save points
		int nb_written = snprintf(buffer, sizeof(buffer),
				"%f ; %f ; %f ; 0.0\r\n",
				poly._line[i].lat,
				poly._line[i].lon,
				(float)poly._line[i].rtime);

		f_write(&g_fileObject, buffer, nb_written, NULL);
	}

	error = f_close(&g_fileObject);
	APP_ERROR_CHECK(error);
	if (error)
	{
		NRF_LOG_ERROR("sd_save_bin_seg close file failed.");
		return;
	}

	NRF_LOG_INFO("sd_save_bin_seg Saved as %s", f_name.c_str());
}


void sd_save_bin_seg(UploadSegment &cur_u_seg) {

	if (!is_fat_init()) return;

	static uint16_t nb = 0;

	char fname[20];
	snprintf(fname, sizeof(fname), "SEG_%04X.TXT", nb++);
	fname[12] = 0;

	if (!cur_u_seg.total_size) {
		NRF_LOG_ERROR("sd_save_bin_seg empty file");
	}

	FIL g_fileObject;
	FRESULT error = f_open(&g_fileObject, fname, FA_WRITE | FA_CREATE_ALWAYS);
	APP_ERROR_CHECK(error);
	if (error)
	{
		NRF_LOG_ERROR("sd_save_bin_seg open file failed.");
		return;
	}

	f_write(&g_fileObject, cur_u_seg.buffer, cur_u_seg.total_size, NULL);

	error = f_close(&g_fileObject);
	APP_ERROR_CHECK(error);
	if (error)
	{
		NRF_LOG_ERROR("sd_save_bin_seg close file failed.");
		return;
	}

	NRF_LOG_INFO("sd_save_bin_seg Saved as %s", fname);

}
#define INCREMENT_INDEX(INCR)              do { index += (INCR); if (index > cur_u_seg.cur_size) return; } while(0)

static void _handler_segment_parse(UploadSegment& cur_u_seg) {

	uint32_t index = 0;

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
	NRF_LOG_INFO("TOT size : %lu", cur_u_seg.total_size);

	uint32_t dist = _decode_uint32_little(cur_u_seg.buffer+index);
	INCREMENT_INDEX(4);
	NRF_LOG_INFO("dist : %lu", dist);

	int32_t i_lat = (int32_t)_decode_uint32_little(cur_u_seg.buffer+index) / 119;
	INCREMENT_INDEX(4);
	NRF_LOG_INFO("i_lat: %ld", i_lat);

	int32_t i_lon = (int32_t)_decode_uint32_little(cur_u_seg.buffer+index) / 119;
	INCREMENT_INDEX(4);
	NRF_LOG_INFO("i_lon: %ld", i_lon);

	i_lat = (int32_t)_decode_uint32_little(cur_u_seg.buffer+index) / 119;
	INCREMENT_INDEX(4);
	NRF_LOG_INFO("i_lat: %ld", i_lat);

	i_lon = (int32_t)_decode_uint32_little(cur_u_seg.buffer+index) / 119;
	INCREMENT_INDEX(4);
	NRF_LOG_INFO("i_lon: %ld", i_lon);
//
//	int16_t grade = (int16_t)_decode_uint16_little(cur_u_seg.buffer+index);
//	INCREMENT_INDEX(2);
//	NRF_LOG_INFO("grade * 10: %d", grade * 10 / 255);

	INCREMENT_INDEX(6); // padding 0

	// dump name
	uint16_t name_length = cur_u_seg.buffer[index];
	INCREMENT_INDEX(1);

	NRF_LOG_INFO("name_length: %u", name_length);

	_dump_as_char(cur_u_seg.buffer+index, name_length); // name
	INCREMENT_INDEX(name_length);

	uint16_t poly_length = _decode_uint16_little(cur_u_seg.buffer+index);
	INCREMENT_INDEX(2);

	NRF_LOG_INFO("Polyline length: %u", poly_length);

	cur_u_seg.poly_index = index;

	INCREMENT_INDEX(poly_length);

	// pr time
	uint16_t pr_time =  _decode_uint16_little(cur_u_seg.buffer+index);
	INCREMENT_INDEX(2);

	// kom time
	uint16_t kom_time =  _decode_uint16_little(cur_u_seg.buffer+index);
	INCREMENT_INDEX(2);

	NRF_LOG_INFO("PR/KOM times: %u %u", pr_time, kom_time);

	INCREMENT_INDEX(2); // hazardous

	uint16_t bin_length =  _decode_uint16_little(cur_u_seg.buffer+index);
	INCREMENT_INDEX(2);

	NRF_LOG_INFO("bin_length: %u", bin_length);

//	uint16_t comp_dist_length = _decode_uint16_little(cur_u_seg.buffer+index);
//	INCREMENT_INDEX(2);
//
//	NRF_LOG_INFO("Time stream length: %u", comp_dist_length);
//
//	cur_u_seg.dist_index = index;
//	INCREMENT_INDEX(comp_dist_length);

	uint16_t stream_length = _decode_uint16_little(cur_u_seg.buffer+index);
	INCREMENT_INDEX(2);

	NRF_LOG_INFO("Effort stream length: %u", stream_length);

	cur_u_seg.effort_index = index;

	INCREMENT_INDEX(bin_length);

//	uint16_t nb_padding = ((words_comp-1) << 2) - (comp_dist_length + comp_stream_length);
//	NRF_LOG_INFO("Nb padding: %u", nb_padding);
//	INCREMENT_INDEX(nb_padding);

	INCREMENT_INDEX(4);

	uint16_t allocate_length = _decode_uint16_little(cur_u_seg.buffer+index);
	INCREMENT_INDEX(2);
	NRF_LOG_INFO("allocate_length: %u", allocate_length);

	uint16_t crc16 = 0;
    for (uint16_t i5 = 0; i5 < index; i5++) {
    	crc16 = FitCRC_Get16(crc16, cur_u_seg.buffer[i5]);
    }
	NRF_LOG_INFO("CRC16 : %lu vs %lu", crc16, _decode_uint16_little(cur_u_seg.buffer + index));

	if (crc16 == _decode_uint16_little(cur_u_seg.buffer + index)) {

		index = cur_u_seg.poly_index;
		PolyLine myPoly;

		NRF_LOG_HEXDUMP_DEBUG(cur_u_seg.buffer+index, poly_length);
		//_dump_as_char(cur_u_seg.buffer+index, poly_length);
		{
			ByteBuffer bBuffer;
			bBuffer.addU(cur_u_seg.buffer+index, poly_length);

			(void)myPoly.decodeBinaryPolyline(bBuffer);
			myPoly.toString();
		}

//		index = cur_u_seg.dist_index;
//
//		if (0) {
//			NRF_LOG_HEXDUMP_DEBUG(cur_u_seg.buffer+index, comp_dist_length);
//			ByteBuffer bBuffer;
//			bBuffer.addU(cur_u_seg.buffer+index, comp_dist_length);
//			SequenceUnpacker unpacker(bBuffer, comp_dist_length);
//			unpacker.unpack();
//			for (size_t i=0; i < unpacker.original.size(); i++) {
//				printf("Ddist:  %ld \n", (int32_t)unpacker.original[i]);
//			}
//		}

		index = cur_u_seg.effort_index;

		NRF_LOG_HEXDUMP_DEBUG(cur_u_seg.buffer+index, stream_length);
		//_dump_as_uint(cur_u_seg.buffer+index, comp_stream_length);
		//_dump_as_char(cur_u_seg.buffer+index, comp_stream_length);
		ByteBuffer bBuffer;
		bBuffer.addU(cur_u_seg.buffer+index, bin_length);
		SequenceUnpacker unpacker(bBuffer, stream_length);
		unpacker.unpack();
		NRF_LOG_INFO("SequenceUnpacker original size %lu", unpacker.original.size());
		int32_t tot_time = (int32_t)unpacker.original[unpacker.original.size()-1] - (int32_t)unpacker.original[0];
		NRF_LOG_INFO("TOT time = %lds vs. PR %u KOM %u", tot_time, pr_time, kom_time);

		for (uint16_t i=0; i < myPoly._line.size() && unpacker.original.size(); i++) {

			myPoly._line[i].rtime = unpacker.original[i];
		}

		sd_save_seg(myPoly);
		sd_save_bin_seg(cur_u_seg);

	} else {

		NRF_LOG_INFO("CRC16 error : %lu vs %lu", crc16, _decode_uint16_little(cur_u_seg.buffer + index));
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
