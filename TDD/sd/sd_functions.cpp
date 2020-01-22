/*
 * sd_functions.cpp
 *
 *  Created on: 18 oct. 2017
 *      Author: Vincent
 */

#include <stdio.h>
#include <string.h>
#include "Model.h"
#include "file_parser.h"
#include "WString.h"
#include "ff.h"
#include <dirent.h>
#include <stdio.h>
#include "sd_hal.h"
#include "sd_functions.h"

#include "segger_wrapper.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* buffer size (in byte) for read/write operations */
#define BUFFER_SIZE (128U)

/*******************************************************************************
 * Variables
 ******************************************************************************/

static TCHAR g_bufferWrite[BUFFER_SIZE]; /* Write buffer */
static TCHAR g_bufferRead[BUFFER_SIZE];  /* Read buffer */

static FIL* g_fileObject;   /* File object */
static FIL* g_EpoFileObject;   /* File object */

/*!
 * @brief Main function
 */
int init_liste_segments(void)
{
	FRESULT error = FR_OK;
	FILINFO* fileInformation;
	uint16_t nb_files_errors = 0;

	if (!is_fat_init()) return -2;

	W_SYSVIEW_OnTaskStartExec(SD_ACCESS_TASK);

	mes_segments._segs.clear();

#if (_FS_RPATH >= 2U)
	error = f_chdrive((char const *)&driverNumberBuffer[0U]);
	if (error)
	{
		LOG_INFO("Change drive failed.");
		W_SYSVIEW_OnTaskStopExec(SD_ACCESS_TASK);
		return -1;
	}
#endif

	LOG_INFO("List the file in the root directory......");

	struct dirent *de;  // Pointer for directory entry

	// opendir() returns a pointer of DIR type.
	DIR *dr = opendir("./../TDD/DB/");

	if (dr == NULL)  // opendir returns NULL if couldn't open directory
	{
		LOG_ERROR("Could not open current directory" );
		return -1;
	}

	do
	{
		//error = f_readdir(directory, &fileInformation);
		fileInformation = readdir(dr);

		/* To the end. */
		if (!fileInformation)
		{
			break;
		}
		if ((error != FR_OK) || (fileInformation->d_name[0U] == 0U))
		{
			LOG_ERROR("f_readdir failure %u", error);
			break;
		}
		if (!fileInformation->d_name[0]) {
			continue;
		}
		if (fileInformation->d_name[0] == '.')
		{
			continue;
		}

		LOG_INFO("Directory file : %s", fileInformation->d_name);

		{
			fileInformation->d_name[12] = 0;

			LOG_DEBUG("General file : %s", (uint32_t)fileInformation->d_name);

			if (Segment::nomCorrect(fileInformation->d_name)) {
				LOG_DEBUG("Segment added : %s", (uint32_t)fileInformation->d_name);
//				LOG_INFO("Segment added");
				mes_segments.push_back(Segment(fileInformation->d_name));
			} else if (Parcours::nomCorrect(fileInformation->d_name)) {
				// pas de chargement en double
				LOG_INFO("Parcours added");
				mes_parcours.push_back(Parcours(fileInformation->d_name));
			} else {
				LOG_INFO("File refused");
				nb_files_errors++;
			}

			NRF_LOG_FLUSH();

		}

	} while (fileInformation->d_name[0U]);

	closedir(dr);

	LOG_INFO("%u files refused", nb_files_errors);
	LOG_INFO("%u segments addded", mes_segments.size());

	NRF_LOG_FLUSH();

	W_SYSVIEW_OnTaskStopExec(SD_ACCESS_TASK);

	return 0;
}


/*!
 * @brief Main function
 */
void uninit_liste_segments(void)
{

	mes_segments._segs.clear();

	mes_parcours._parcs.clear();

	return;
}

/**
 * Loads a segment from the SD card
 * @param seg The reference of this segment
 * @return -1 if failure, else the number of points loaded
 */
int load_segment(Segment& seg) {

	int res = 0;
	float time_start = 0.;

	if (!is_fat_init()) return -2;

	time_start = 0.;

	W_SYSVIEW_OnTaskStartExec(SD_ACCESS_TASK);

	String fat_name = "./../TDD/DB/";
	fat_name += seg.getName();

	//error = f_open(g_fileObject, _T(fat_name.c_str()), FA_READ);
	g_fileObject = fopen(_T(fat_name.c_str()), "r");

	//if (error) error = f_open(&g_fileObject, _T(fat_name.c_str()), FA_READ);
	if (!g_fileObject)
	{
		LOG_ERROR("Open file %s failed", _T(fat_name.c_str()));
		W_SYSVIEW_OnTaskStopExec(SD_ACCESS_TASK);
		return -1;
	}

	memset(g_bufferRead, 0U, sizeof(g_bufferRead));

	// allocate segment memory
	if (!seg.init()) return -1;

	while (fgets(g_bufferRead, sizeof(g_bufferRead)-1, g_fileObject)) {

		if (strstr(g_bufferRead, "<")) {
			// meta data
		} else if (strstr(g_bufferRead, ";")) {
			// on est pret a charger le point
			if (!chargerPointSeg(g_bufferRead, seg, time_start))
				res++;

			memset(g_bufferRead, 0, sizeof(g_bufferRead));
		}

		if (check_memory_exception()) return -1;
	}

	int error = fclose (g_fileObject);
	if (error)
	{
		NRF_LOG_ERROR("Close file failed. (error %u)", error);
		W_SYSVIEW_OnTaskStopExec(SD_ACCESS_TASK);
		return -1;
	}

	W_SYSVIEW_OnTaskStopExec(SD_ACCESS_TASK);

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
	TCHAR g_bufferReadPRC[BUFFER_SIZE];  /* Read buffer */
	FIL* g_fileObjectPRC;   /* File object */

	if (!is_fat_init()) return -2;

	// clear list
	mon_parcours.desallouerPoints();

	String fat_name = "./../TDD/DB/";
	fat_name += mon_parcours.getName();

	W_SYSVIEW_OnTaskStartExec(SD_ACCESS_TASK);

	g_fileObjectPRC = fopen(_T(fat_name.c_str()), "r");

	if (!g_fileObjectPRC)
	{
		LOG_INFO("Open PRC %s failed", _T(fat_name.c_str()));
		return -1;
	}

	memset(g_bufferReadPRC, 0U, sizeof(g_bufferReadPRC));

	while (fgets(g_bufferReadPRC, sizeof(g_bufferReadPRC)-1, g_fileObjectPRC)) {

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

	int error = fclose(g_fileObjectPRC);
	if (error)
	{
		LOG_INFO("Close file failed.");
		return -1;
	} else {
		LOG_INFO("%u points added to PRC", res);
	}

	return res;
}

/**
 * desalloue automatiquement les segments lointains
 * alloue automatiquement les segments proches
 *
 * @param mon_seg
 * @param lat1
 * @param long1
 * @return
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
//				Serial.println(F("Echec parsing du nom"));
//				loggerMsg("Echec parsing du nom");
//				loggerMsg(mon_seg->getName());
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
			mon_seg.setStatus(SEG_OFF);

//			display.notifyANCS(1, "WTCH", "Seg trop loin");
		}


	}

	return ret_val;
}

/**
 *
 * @param att
 * @param nb_pos
 */
void sd_save_pos_buffer(SAttTime* att, uint16_t nb_pos) {

	uint32_t millis_ = millis();

//	FRESULT error = f_open(&g_fileObject, "histo.txt", FA_OPEN_APPEND | FA_WRITE);
//	if (error) error = f_open(&g_fileObject, "histo.txt", FA_OPEN_APPEND | FA_WRITE);
	g_fileObject = fopen("histo.txt", "a+");

	if (!g_fileObject)
	{
		LOG_INFO("Open histo failed");
		return;
	}

	for (size_t i=0; i< nb_pos; i++) {
		uint16_t to_wr = snprintf(g_bufferWrite, sizeof(g_bufferWrite), "%f;%f;%f;%u;%d\r\n",
				att[i].loc.lat, att[i].loc.lon,
				att[i].loc.alt, att[i].date.secj,
				att[i].pwr);

		(void)fwrite(g_bufferWrite, to_wr, 1, g_fileObject);

		perform_system_tasks_light();
	}

	int error = fclose(g_fileObject);
	if (error)
	{
		LOG_INFO("Close file failed.");
		return;
	} else {
		LOG_INFO("Points added to histo: %u %u", nb_pos, millis() - millis_);
	}
}


bool sd_erase_pos(void) {

//	FRESULT error = f_unlink("histo.txt");
//	if (error)
//	{
//		LOG_INFO("Unlink file failed.");
//		return false;
//	}

	return true;
}

/**
 *
 * @return The size of the EPO file
 */
int epo_file_size(void) {

	FRESULT error;
	const char* fname = "./../TDD/DB/MTK14.EPO";

	if (!is_fat_init()) return -1;

	FILE *p_file = NULL;
	p_file = fopen(fname,"rb");
	if (!p_file) return 0;

	fseek(p_file,0,SEEK_END);
	int size = ftell(p_file);
	fclose(p_file);

	return size;
}

/**
 *
 * @return
 */
bool epo_file_start(int current_gps_hour_) {

	FRESULT error;
	const char* fname = "./../TDD/DB/MTK14.EPO";

	if (!is_fat_init()) return -1;

	g_EpoFileObject = fopen(fname, "r");
	if (!g_EpoFileObject)
	{
		LOG_INFO("Open file failed.");
		return false;
	}

	int gps_hour = 0;

	UINT size_read = fread (
			&gps_hour,	        /* Pointer to data buffer */
			sizeof(uint32_t),   /* Number of bytes to read */
			sizeof(gps_hour),   /* Buffer size */
			g_EpoFileObject
	);
	if (!size_read)
	{
		LOG_INFO("Read file failed.");
		return false;
	}

	gps_hour &= 0x00FFFFFF;

	// determine the segment to use
	int segment = (current_gps_hour_ - gps_hour) / 6;
	if ((segment < 0) || (segment >= EPO_SAT_SEGMENTS_NUM))
	{
		return false;
	}

	return (FR_OK == fseek(g_EpoFileObject, segment*(EPO_SAT_DATA_SIZE_BYTES)*(EPO_SAT_SEGMENTS_NB), SEEK_SET));

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

	UINT size_read = fread (
			g_bufferRead,	    /* Pointer to data buffer */
			size_,              /* Number of bytes to read */
			BUFFER_SIZE,	    /* Buffer size */
			g_EpoFileObject
	);
	if (!size_read)
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

	int error = fclose (g_EpoFileObject);
	if (error)
	{
		LOG_INFO("Close file failed.");
		return -1;
	}

//#ifndef DEBUG_CONFIG
//	if (toBeDeleted) {
//		error = funlink("./../TDD/MTK14.EPO");
//		if (error)
//		{
//			LOG_INFO("Unlink file failed.");
//			return -2;
//		}
//	}
//#endif

	return 0;
}
