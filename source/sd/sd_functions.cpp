/*
 * sd_functions.cpp
 *
 *  Created on: 18 oct. 2017
 *      Author: Vincent
 */

#include <stdio.h>
#include <string.h>
#include "boards.h"
#include "Model.h"
#include "millis.h"
#include "file_parser.h"
#include "nrf_delay.h"
#include "sd_hal.h"
#include "WString.h"
#include "segger_wrapper.h"

#include "ff.h"
#include "sd_functions.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* buffer size (in byte) for read/write operations */
#define BUFFER_SIZE (192U)

#define HISTO_MARKER_CHAR          '@'

/*******************************************************************************
 * Variables
 ******************************************************************************/

static TCHAR g_bufferWrite[BUFFER_SIZE]; /* Write buffer */
static TCHAR g_bufferRead[BUFFER_SIZE];  /* Read buffer */

static TCHAR g_bufferReadPRC[BUFFER_SIZE];  /* Read buffer */

static FIL g_EpoFileObject;   /* File object */
static FIL g_LogFileObject;   /* File object */

static eSDTaskQuery m_cur_query = eSDTaskQueryNone;

class HistoFile {
public:
	HistoFile(const char * const name) {
		_name = name;
		fsize = 0;
	};
	HistoFile(const char * const name, uint32_t length) {
		_name = name;
		fsize = length;
	};


	String _name;
	uint32_t fsize;
private:
};

static std::vector<HistoFile> m_list_fit;
static std::vector<HistoFile> m_list_histo;
static uint16_t cur_idx = 0;


/**
 *
 */
int sd_functions__start_query(eSDTaskQuery query, const char * const fname, uint32_t *f_size) {

	LOG_INFO("SD function query start !");

	// reset query
	if (m_cur_query != eSDTaskQueryNone) {

		LOG_INFO("SD function query already running...");

		int ret = sd_functions__stop_query();
		if (ret) return ret;
	}

	switch (query) {

	case eSDTaskQueryNone: {

	} break;

	case eSDTaskQueryFit:
	{
		if (f_size) {
			// get out file size
			for (auto fit_file : m_list_fit) {

				if (fit_file._name.equals(fname) &&
						fit_file.fsize > 0) {
					*f_size = fit_file.fsize;

					int ret = 0;
					if ((ret = sd_functions__query_file_start(fname)) != 0) {
						return ret;
					}
					LOG_INFO("SD function query FIT file");
				}
			}
		}
	} break;

	case eSDTaskQueryFile: {

		int ret = 0;
		if ((ret = sd_functions__query_file_start(fname)) != 0) {
			return ret;
		}
		LOG_INFO("SD function query file");
	} break;

	case eSDTaskQueryHisto: {
		if (m_list_histo.size() == 0) {
			return -2;
		}
		LOG_INFO("SD function query histo");
	} break;

	case eSDTaskQueryDelete: {
		int ret = 0;
		ret = sd_functions__unlink(fname);
		m_cur_query = eSDTaskQueryNone;
		LOG_INFO("SD function unlink res=%d", ret);
		return ret;
	} break;

	default:
		m_cur_query = eSDTaskQueryNone;
		return -1;
		break;

	}

	// set query
	m_cur_query = query;

	return 0;
}

/**
 *
 */
int sd_functions__stop_query(void) {

	int ret = 0;

	switch (m_cur_query) {

	case eSDTaskQueryNone: {

	} break;

	case eSDTaskQueryFit:
	case eSDTaskQueryFile: {
		if (sd_functions__query_file_stop(false)) {
			ret = -2;
		}
	} break;

	case eSDTaskQueryHisto: {
		cur_idx = 0;
	} break;

	case eSDTaskQueryDelete: {

	} break;

	default:
		ret = -1;
		break;

	}

	LOG_INFO("SD function query stop");

	m_cur_query = eSDTaskQueryNone;

	return ret;
}

/**
 *
 */
int sd_functions__run_query(int restart, sCharArray *p_array, size_t max_size) {

	int ret = 0;

	p_array->length = 0;

	switch (m_cur_query) {

	case eSDTaskQueryNone: {

	} break;

	case eSDTaskQueryFit:
	case eSDTaskQueryFile: {
		if (!sd_functions__query_file_run(p_array, max_size)) {
			return -2;
		}
	} break;

	case eSDTaskQueryHisto: {
		if (!sd_functions__query_histo_list(restart, p_array, max_size)) {
			return -2;
		}
	} break;

	case eSDTaskQueryDelete: {
		m_cur_query = eSDTaskQueryNone;
	} break;

	default:
		LOG_ERROR("sd_functions__run_query: NO query started !");
		m_cur_query = eSDTaskQueryNone;
		return -1;
		break;

	}

	return ret;
}

/**
 *
 */
uint16_t sd_functions__query_fit_list(int restart, sCharArray *p_array, size_t max_size) {

	if (!is_fat_init()) return 0;

	sysview_task_void_enter(SdFunction);

	size_t nb_files = 0;
	size_t rem_size = 0;
	size_t cur_size = 0;

	if (restart) {
		cur_idx = 0;
		// number of FIT files
		cur_size = 1;

	}

	while (cur_idx < m_list_fit.size() &&
			cur_size + 4 < max_size) {

		if (m_list_fit[cur_idx].fsize > 0) {
			char tab[15];
			memset(tab, 0, sizeof(tab));
			m_list_fit[cur_idx]._name.toCharArray(tab, sizeof(tab));
			tab[8] = 0;
			uint32_t l_value = strtoul(tab, NULL, 16);

			NRF_LOG_INFO("Adding FIT %08lX size %lu", l_value, m_list_fit[cur_idx].fsize);

			encode_uint32((uint8_t*)p_array->str + cur_size, l_value);

			cur_size += 4;
			nb_files += 1;
		}
		cur_idx  += 1;
	}

	p_array->str[0] = nb_files;

	rem_size = 4 * m_list_fit.size() - 4 * cur_idx;

	p_array->length = cur_size;

	sysview_task_void_exit(SdFunction);

	return rem_size;
}

/**
 *
 */
uint16_t sd_functions__query_histo_list(int restart, sCharArray *p_array, size_t max_size) {

	if (!is_fat_init()) return 0;

	sysview_task_void_enter(SdFunction);

	size_t tot_size = 0;
	size_t cur_size = 0;

	if (restart) {
		cur_idx = 0;
	}

	tot_size = 12 * m_list_histo.size();

	if (tot_size == 0) {
		return 0;
	}

	while (cur_idx < m_list_histo.size() &&
			cur_size + m_list_histo[cur_idx]._name.length() + 2 < max_size) {

		m_list_histo[cur_idx]._name.toCharArray(p_array->str + cur_size, max_size - cur_size);

		LOG_INFO("Adding histo size %u", m_list_histo[cur_idx]._name.length());

		cur_size += m_list_histo[cur_idx]._name.length();
		cur_idx  += 1;

		if (cur_idx == m_list_histo.size()) {

			// add a line termination
			p_array->str[cur_size++] = '\r';
			p_array->str[cur_size++] = '\n';
		}
	}

	p_array->length = cur_size;

	sysview_task_void_exit(SdFunction);

	return tot_size;
}

/*!
 * @brief Main function
 */
int init_liste_segments(void)
{
	FRESULT error;
	DIR *p_dir = nullptr;
#ifndef TDD
	DIR directory; /* Directory object */
	p_dir = &directory;
#endif
	FILINFO fileInformation;
	uint16_t nb_files_errors = 0;

	if (!is_fat_init()) return -2;

	sysview_task_void_enter(SdFunction);

	mes_segments._segs.clear();

#if (_FS_RPATH >= 2U)
	error = f_chdrive((char const *)&driverNumberBuffer[0U]);
	if (error)
	{
		LOG_INFO("Change drive failed.");
		sysview_task_void_exit();
		return -1;
	}
#endif

	LOG_INFO("List the file in the root directory......");
	if (f_opendir(p_dir, "/"))
	{
		LOG_INFO("Open directory failed.");
		sysview_task_void_exit(SdFunction);
		return -1;
	}

	do
	{
		error = f_readdir(p_dir, &fileInformation);

		/* To the end. */
		if ((error != FR_OK) || (fileInformation.fname[0U] == 0U))
		{
			break;
		}
		if (!fileInformation.fname[0]) {
			continue;
		}
		if (fileInformation.fname[0] == '.')
		{
			continue;
		}
		if (fileInformation.fattrib & AM_DIR)
		{
//			LOG_INFO("Directory file : %s", (uint32_t)fileInformation.fname);
		}
		else
		{
			fileInformation.fname[12] = 0;

			LOG_DEBUG("General file : %s", (uint32_t)fileInformation.fname);

			if (Segment::nomCorrect(fileInformation.fname)) {

				LOG_DEBUG("Segment added : %s", (uint32_t)fileInformation.fname);
				mes_segments.push_back(Segment(fileInformation.fname));

			} else if (Parcours::nomCorrect(fileInformation.fname)) {

				// pas de chargement en double
				LOG_INFO("Parcours added");
				mes_parcours.push_back(Parcours(fileInformation.fname));

			} else if (fileInformation.fname[0] == HISTO_MARKER_CHAR) {

				LOG_INFO("Histo added");
				HistoFile h_file(fileInformation.fname, fileInformation.fsize);
				m_list_histo.push_back(h_file);

			} else if (memcmp(&fileInformation.fname[8], (const char *)".FIT ", 4)==0) {

				LOG_INFO("FIT added %s size %lu", fileInformation.fname, fileInformation.fsize);
				HistoFile h_file(fileInformation.fname, fileInformation.fsize);
				m_list_fit.push_back(h_file);

			} else {

				LOG_INFO("File refused");
				nb_files_errors++;
			}

			NRF_LOG_FLUSH();

		}

	} while (fileInformation.fname[0U]);

	if (f_closedir(p_dir))
	{
		LOG_INFO("Close directory failed.");
		sysview_task_void_exit(SdFunction);
		return -3;
	}

	LOG_INFO("%u files refused", nb_files_errors);
	LOG_INFO("%u segments addded", mes_segments.size());

	NRF_LOG_FLUSH();

	sysview_task_void_exit(SdFunction);

	return 0;
}


/*!
 * @brief Main function
 */
void uninit_liste_segments(void)
{

	mes_segments._segs.clear();

	mes_parcours._parcs.clear();

	m_list_histo.clear();

	return;
}

/**
 * Loads a segment from the SD card
 * @param seg The reference of this segment
 * @return -1 if failure, else the number of points loaded
 */
int load_segment(Segment& seg) {

	int res = 0;
	FRESULT error;
	float time_start = 0.;

	if (!is_fat_init()) return -2;

	time_start = 0.;

	sysview_task_void_enter(SdFunction);

	String fat_name = seg.getName();

	FIL g_fileObject;   /* File object */
	error = f_open(&g_fileObject, _T(fat_name.c_str()), FA_READ);
	if (error)
	{
		LOG_ERROR("Open file failed. (error %u)", error);
		sysview_task_void_exit(SdFunction);
		return -1;
	}

	memset(g_bufferRead, 0U, sizeof(g_bufferRead));

	// allocate segment memory
	if (!seg.init()) return -1;

	while (f_gets(g_bufferRead, sizeof(g_bufferRead)-1, &g_fileObject)) {

		if (strstr(g_bufferRead, "<")) {
			// meta data
		} else if (strstr(g_bufferRead, ";")) {
			// on est pret a charger le point
			if (!chargerPointSeg(g_bufferRead, seg, time_start)) {
				res++;
#if defined( DEBUG_NRF_USER ) || defined( TDD )
				if (res == 1 &&
						seg.getFirstPoint()) {

					float tmp_lat, tmp_lon, dist;
					parseSegmentName(seg.getName(), &tmp_lat, &tmp_lon);
					dist = seg.getFirstPoint()->dist(tmp_lat, tmp_lon);

					LOG_INFO("Distance to seg name: %d ", (int)dist);
				}
#endif
			}
		}

		if (check_memory_exception()) return -1;
	}

	error = f_close (&g_fileObject);
	if (error)
	{
		LOG_ERROR("Close file failed. (error %u)", error);
		sysview_task_void_exit(SdFunction);
		return -1;
	}

	sysview_task_void_exit(SdFunction);

	LOG_INFO("%d points loaded", res);

	return res;
}

/**
 *
 * @param mon_parcours
 * @return -1 if failure, the number of points otherwise
 */
int load_parcours(Parcours& mon_parcours) {

	int res = 0;
	FRESULT error;

	if (!is_fat_init()) return -2;

	// clear list
	mon_parcours.desallouerPoints();

	String fat_name = mon_parcours.getName();

	sysview_task_void_enter(SdFunction);

	FIL g_fileObjectPRC;   /* File object */
	error = f_open(&g_fileObjectPRC, _T(fat_name.c_str()), FA_READ);

	if (error)
	{
		LOG_INFO("Open file failed.");
		sysview_task_void_exit(SdFunction);
		return -1;
	}

	memset(g_bufferReadPRC, 0U, sizeof(g_bufferReadPRC));

	while (f_gets(g_bufferReadPRC, sizeof(g_bufferReadPRC)-1, &g_fileObjectPRC)) {

		// on se met au bon endroit
		if (strstr(g_bufferReadPRC, "<")) {
			// meta data
		} else if (strstr(g_bufferReadPRC, " ")) {
			// on est pret a charger le point
			if (!chargerPointPar(g_bufferReadPRC, mon_parcours))
				res++;
		}

		if (check_memory_exception()) return -1;

		// continue to perform the critical system tasks
		w_task_yield();

	} // fin du fichier

	error = f_close(&g_fileObjectPRC);
	if (error)
	{
		LOG_INFO("Close file failed.");
		sysview_task_void_exit(SdFunction);
		return -1;
	} else {
		LOG_INFO("%u points added to PRC", res);
	}

	sysview_task_void_exit(SdFunction);

	return res;
}

/**
 * desalloue automatiquement les segments lointains
 * alloue automatiquement les segments proches
 *
 * @param mon_seg
 * @param lat1
 * @param long1
 * @return distance to segment if success, negative number if error
 */
float segment_allocator(Segment& mon_seg, float lat1, float long1) {

	static float tmp_dist = 0.;
	static float tmp_lat = 0.;
	static float tmp_lon = 0.;
	float ret_val = 5000;

	if (!is_fat_init()) return ret_val;

	// le segment est rempli
	if (mon_seg.isValid() && mon_seg.getStatus() == SEG_OFF) {

		if (mon_seg.longueur() >= 1) {
			// on teste l'eloignement
			Point pp = *mon_seg.getFirstPoint();

			tmp_dist = distance_between(lat1, long1, pp._lat, pp._lon);
			ret_val = tmp_dist;

			if (tmp_dist > DIST_ALLOC) {
				// on desalloue
				LOG_INFO("Unallocate %s", mon_seg.getName());

				mon_seg.setStatus(SEG_OFF);
				mon_seg.uninit();
			}
		}
		else {

			if (parseSegmentName(mon_seg.getName(), &tmp_lat, &tmp_lon) == 1) {
				LOG_ERROR("Echec parsing du nom");
				return ret_val;
			}

			// on etudie si on doit charger
			tmp_dist = distance_between(lat1, long1, tmp_lat, tmp_lon);
			ret_val = tmp_dist;

			if (tmp_dist < DIST_ALLOC) {

				if (mon_seg.longueur() > 0) {
					LOG_INFO("Saving points %s", mon_seg.getName());

					mon_seg.desallouerPoints();
				}

				int res = load_segment(mon_seg);
				LOG_INFO("-->> Loading segment %s %d", mon_seg.getName(), res);

				if (res <= 0) return res;

				mon_seg.init();
			}
		}

	} else if (mon_seg.longueur() > 0 && mon_seg.isValid()) {

		// on teste l'eloignement au premier point
		Point pp = *mon_seg.getFirstPoint();
		tmp_dist = distance_between(lat1, long1, pp._lat, pp._lon);
		ret_val = tmp_dist;

		// distance au segment
		Point tmp_pt(lat1, long1, 0., 0.);
		tmp_dist = mon_seg.dist(&tmp_pt);

		// test desallocation
		if (tmp_dist > MARGE_DESACT * DIST_ALLOC) {
			// on desalloue
			LOG_INFO("Desallocation non nominale !");

			mon_seg.desallouerPoints();
			mon_seg.uninit();
			mon_seg.setStatus(SEG_OFF);
		}


	}

	return ret_val;
}

/**
 *
 * @param att
 * @param nb_pos
 */
void sd_save_pos_buffer(SAttTime att[], uint16_t nb_pos) {

	fit_save_pos_buffer(att, nb_pos);

	String fname = HISTO_MARKER_CHAR;
	fname += att->date.date;
	fname += ".txt";

	FIL g_fileObject;
	FRESULT error = f_open(&g_fileObject, fname.c_str(), FA_OPEN_APPEND | FA_WRITE);
	APP_ERROR_CHECK(error);
	if (error)
	{
		LOG_INFO("Open file failed.");
		return;
	}

	for (uint16_t i=0; i< nb_pos; i++) {

		// print histo
		int to_wr = snprintf(g_bufferWrite, sizeof(g_bufferWrite),
				"%f;%f;%.2f;%lu;"
				"%d;%u;%lu;"
				"%.3f;%.3f;%.3f;%.2f;"
				"%.1f;%.2f;%.2f;%.2f;"
				"%.0f;%.0f;%.0f;"
				"%.1f;"
				"\r\n",
				att[i].loc.lat, att[i].loc.lon, att[i].loc.alt, att[i].date.secj,
				att[i].sensors.pwr, att[i].sensors.bpm, att[i].sensors.cadence,
				att[i].alti.alpha_bar, att[i].alti.alpha_zero, att[i].alti.baro_ele, att[i].alti.baro_corr,
				att[i].alti.climb, att[i].alti.filt_ele, att[i].alti.gps_ele, att[i].alti.vit_asc,
				att[i].alti.rough[0], att[i].alti.rough[1], att[i].alti.rough[2],
				att[i].alti.b_rough
				);

		if (to_wr > 0 && to_wr < (int)sizeof(g_bufferWrite)) {
			f_write(&g_fileObject, g_bufferWrite, to_wr, NULL);
		} else {
			APP_ERROR_CHECK(0x2);
		}

	}

	error = f_close(&g_fileObject);
	APP_ERROR_CHECK(error);
	if (error)
	{
		LOG_INFO("Close file failed.");
		return;
	} else {
		LOG_INFO("Points added to histo: %u", nb_pos);
	}

}

/**
 *
 * @return The size of the EPO file
 */
int epo_file_size(void) {

	FRESULT error;
	const char* fname = "MTK14.EPO";

	if (!is_fat_init()) return -1;

	FILINFO file_info;
	error = f_stat (fname, &file_info);
	if (error) error = f_stat (fname, &file_info);
	if (error) {
		LOG_INFO("Stat file failed.");
		return -1;
	}

	return file_info.fsize;
}

/**
 *
 * @return
 */
bool epo_file_start(int current_gps_hour) {

	FRESULT error;
	const char* fname = "MTK14.EPO";

	if (!is_fat_init()) return -1;

	error = f_open(&g_EpoFileObject, fname, FA_READ);
	if (error)
	{
		LOG_INFO("Open file failed.");
		return false;
	}

	int gps_hour = 0;

	UINT size_read = 0;
	error = f_read (
			&g_EpoFileObject, 	/* Pointer to the file object */
			&gps_hour,	        /* Pointer to data buffer */
			sizeof(uint32_t),   /* Number of bytes to read */
			&size_read	        /* Pointer to number of bytes read */
	);
	if (error)
	{
		LOG_INFO("Read file failed.");
		return false;
	}

	gps_hour &= 0x00FFFFFF;

	// determine the segment to use
	int segment = (current_gps_hour - gps_hour) / 6;
	if ((segment < 0) || (segment >= EPO_SAT_SEGMENTS_NUM))
	{
		return false;
	}

	return (FR_OK == f_lseek(&g_EpoFileObject, segment*(EPO_SAT_DATA_SIZE_BYTES)*(EPO_SAT_SEGMENTS_NB)));
}

/**
 *
 * @param epo_data
 * @return The number of sat_data read, or -1 if error
 */
int epo_file_read(sEpoPacketSatData* sat_data, uint16_t size_) {

	memset(g_bufferRead, 0U, sizeof(g_bufferRead));

	ASSERT(sat_data);

	if (!is_fat_init()) return -1;

	UINT size_read = 0;
	FRESULT error = f_read (
			&g_EpoFileObject, 	/* Pointer to the file object */
			g_bufferRead,	    /* Pointer to data buffer */
			size_,              /* Number of bytes to read */
			&size_read	        /* Pointer to number of bytes read */
	);

	if (error)
	{
		LOG_INFO("Read EPO file failed.");
		return -1;
	}

	if (size_read != size_) {
		LOG_INFO("End of EPO file");
		return 1;
	} else {
		memcpy(sat_data->sat, g_bufferRead, size_);
	}

	return 0;
}

/**
 *
 * @return
 */
int epo_file_stop(bool toBeDeleted) {

	if (!is_fat_init()) return -3;

	FRESULT error = f_close (&g_EpoFileObject);
	if (error)
	{
		LOG_INFO("Close file failed.");
		return -1;
	}

#ifndef DEBUG_CONFIG
	if (toBeDeleted) {
		error = f_unlink("MTK14.EPO");
		if (error)
		{
			LOG_INFO("Unlink file failed.");
			return -2;
		}
	}
#endif

	return 0;
}


/**
 *
 * @return True on success
 */
int sd_functions__query_file_start(const char * const fname) {

	FRESULT error;

	if (!is_fat_init() || !fname) return -1;

	error = f_open(&g_LogFileObject, fname, FA_READ);
	if (error)
	{
		LOG_INFO("Open file failed.");
		return -2;
	}
	LOG_INFO("sd_functions__query_file_start %s", fname);

	return 0;
}

/**
 *
 * @param log_buffer
 * @param size_
 * @return The pointer to read string, or NULL if problem
 */
char* sd_functions__query_file_run(sCharArray *p_array, size_t max_size) {

	if (!is_fat_init()) return NULL;

	sysview_task_void_enter(SdFunction);

	UINT a_size = 0;
	FRESULT error = f_read (
			&g_LogFileObject, 	/* Pointer to the file object */
			p_array->str,	    /* Pointer to data buffer */
			max_size,           /* Number of bytes to read */
			&a_size	/* Pointer to number of bytes read */
	);

	p_array->length = a_size;

	if (error) {

		LOG_INFO("Read LOG file EOF: %d ERR: %u", f_eof(&g_LogFileObject), f_error(&g_LogFileObject));
		sysview_task_void_exit(SdFunction);

		return NULL;
	}

	sysview_task_void_exit(SdFunction);

	return p_array->str;
}

/**
 *
 * @return 0 on success
 */
int sd_functions__query_file_stop(bool toBeDeleted) {

	if (!is_fat_init()) return -3;

	FRESULT error = f_close (&g_LogFileObject);
	if (error)
	{
		LOG_INFO("Close file failed.");
		return -1;
	}

	return 0;
}

/**
 *
 */
int sd_functions__unlink(const char * const fname) {

	if (!is_fat_init() || !fname) return -3;

	// get out file size
	for (auto& fit_file : m_list_fit) {

		if (fit_file._name.equals(fname)) {

			// remove from fit list
			fit_file.fsize = 0;
		}
	}

	FRESULT error = f_unlink(fname);
	if (error)
	{
		LOG_INFO("Unlink file %s failed.", fname);
		return -2;
	}

	return 0;
}
