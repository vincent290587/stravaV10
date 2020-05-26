////////////////////////////////////////////////////////////////////////////////
// The following FIT Protocol software provided may be used with FIT protocol
// devices only and remains the copyrighted property of Dynastream Innovations Inc.
// The software is being provided on an "as-is" basis and as an accommodation,
// and therefore all warranties, representations, or guarantees of any kind
// (whether express, implied or statutory) including, without limitation,
// warranties of merchantability, non-infringement, or fitness for a particular
// purpose, are specifically disclaimed.
//
// Copyright 2008-2015 Dynastream Innovations Inc.
////////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS

#include "stdio.h"
#include "string.h"

#include "fit_product.h"
#include "fit_crc.h"

#include "encode_lib.h"


///////////////////////////////////////////////////////////////////////
// Private Variables
///////////////////////////////////////////////////////////////////////

static FIT_UINT16 data_crc = 0;

///////////////////////////////////////////////////////////////////////
// Private Function Prototypes
///////////////////////////////////////////////////////////////////////


void WriteData(const void *data, FIT_UINT8 data_size)
{
	FIT_UINT8 offset;

	WriteDataBare(data, data_size);

	for (offset = 0; offset < data_size; offset++)
		data_crc = FitCRC_Get16(data_crc, *((FIT_UINT8 *)data + offset));
}

void WriteMessageDefinition(FIT_UINT8 local_mesg_number, const void *mesg_def_pointer, FIT_UINT8 mesg_def_size)
{
	FIT_UINT8 header = local_mesg_number | FIT_HDR_TYPE_DEF_BIT;
	WriteData(&header, FIT_HDR_SIZE);
	WriteData(mesg_def_pointer, mesg_def_size);
}

void WriteMessageDefinitionWithDevFields
(
		FIT_UINT8 local_mesg_number,
		const void *mesg_def_pointer,
		FIT_UINT8 mesg_def_size,
		FIT_UINT8 number_dev_fields,
		FIT_DEV_FIELD_DEF *dev_field_definitions
)
{
	FIT_UINT16 i;
	FIT_UINT8 header = local_mesg_number | FIT_HDR_TYPE_DEF_BIT | FIT_HDR_DEV_DATA_BIT;
	WriteData(&header, FIT_HDR_SIZE);
	WriteData(mesg_def_pointer, mesg_def_size);

	WriteData(&number_dev_fields, sizeof(FIT_UINT8));
	for (i = 0; i < number_dev_fields; i++)
	{
		WriteData(&dev_field_definitions[i], sizeof(FIT_DEV_FIELD_DEF));
	}
}

void WriteMessage(FIT_UINT8 local_mesg_number, const void *mesg_pointer, FIT_UINT8 mesg_size)
{
	WriteData(&local_mesg_number, FIT_HDR_SIZE);
	WriteData(mesg_pointer, mesg_size);
}

void WriteDeveloperField(const void *data, FIT_UINT8 data_size)
{
	WriteData(data, data_size);
}

void WriteFileHeader(void)
{
	FIT_FILE_HDR file_header;

	file_header.header_size = FIT_FILE_HDR_SIZE;
	file_header.profile_version = FIT_PROFILE_VERSION;
	file_header.protocol_version = FIT_PROTOCOL_VERSION_20;
	memcpy((FIT_UINT8 *)&file_header.data_type, ".FIT", 4);

	UserData_GoTo(eUserDataPosEnd);   // fseek (fp , 0 , SEEK_END);
	long f_size = UserData_Ftell();
	UserData_GoTo(eUserDataPosStart); // fseek (fp , 0 , SEEK_SET);

	file_header.data_size = f_size - FIT_FILE_HDR_SIZE - sizeof(FIT_UINT16);
	file_header.crc = FitCRC_Calc16(&file_header, FIT_STRUCT_OFFSET(crc, FIT_FILE_HDR));

	WriteDataBare((void *)&file_header, FIT_FILE_HDR_SIZE);
}


///////////////////////////////////////////////////////////////////////
// Public functions
///////////////////////////////////////////////////////////////////////

int encode_process(sEncodingData * const p_data)
{
	static FIT_UINT8 local_mesg_number = 0;
	static FIT_DATE_TIME start_time = 0;

	// reset recording
	if (p_data->cmd == eEncodingCommandStart) {

		data_crc = 0;
		WriteFileHeader();
	}

	// Write file id message.
	if (p_data->cmd == eEncodingCommandStart) {

		start_time = p_data->timestamp;

		FIT_FILE_ID_MESG file_id;
		Fit_InitMesg(fit_mesg_defs[FIT_MESG_FILE_ID], &file_id);
		file_id.time_created = p_data->timestamp;
		file_id.type = FIT_FILE_ACTIVITY;
		file_id.manufacturer = FIT_MANUFACTURER_LEZYNE;
		strncpy(file_id.product_name, "Frava 310", sizeof(file_id.product_name));
		WriteMessageDefinition(local_mesg_number, fit_mesg_defs[FIT_MESG_FILE_ID], FIT_FILE_ID_MESG_DEF_SIZE);
		WriteMessage(local_mesg_number, &file_id, FIT_FILE_ID_MESG_SIZE);

		FIT_FILE_CREATOR_MESG creator_msg;
		Fit_InitMesg(fit_mesg_defs[FIT_MESG_FILE_CREATOR], &creator_msg);
		creator_msg.hardware_version = 2;
		creator_msg.software_version = 1;
		WriteMessageDefinition(local_mesg_number, fit_mesg_defs[FIT_MESG_FILE_CREATOR], FIT_FILE_CREATOR_MESG_DEF_SIZE);
		WriteMessage(local_mesg_number, &creator_msg, FIT_FILE_CREATOR_MESG_SIZE);

		// Write event
		FIT_EVENT_MESG event_msg;
		Fit_InitMesg(fit_mesg_defs[FIT_MESG_EVENT], &event_msg);
		event_msg.timestamp = p_data->timestamp;
		event_msg.event_type = FIT_EVENT_TYPE_START;
		WriteMessageDefinition(local_mesg_number, fit_mesg_defs[FIT_MESG_EVENT], FIT_EVENT_MESG_DEF_SIZE);
		WriteMessage(local_mesg_number, &event_msg, FIT_EVENT_MESG_SIZE);
	}

	// write record
	if (p_data->cmd == eEncodingCommandNone) {

		FIT_RECORD_MESG record_msg;
		Fit_InitMesg(fit_mesg_defs[FIT_MESG_RECORD], &record_msg);
		record_msg.timestamp = p_data->timestamp;
		record_msg.position_lat  = p_data->lat;
		record_msg.position_long = p_data->lon;
		record_msg.altitude    = 2500 + p_data->alt_cm / 20; // 1000=-300m   / 0=-500m
		record_msg.temperature = p_data->temp_c;
		record_msg.grade       = p_data->grade_pc;
		record_msg.heart_rate  = p_data->hrm;
		record_msg.cadence     = p_data->cad;
		WriteMessageDefinition(local_mesg_number, fit_mesg_defs[FIT_MESG_RECORD], sizeof(FIT_RECORD_MESG_DEF));
		WriteMessage(local_mesg_number, &record_msg, sizeof(FIT_RECORD_MESG));
	}

	// Write event
	if (p_data->cmd == eEncodingCommandStop) {

		// lap
		FIT_LAP_MESG lap_msg;
		Fit_InitMesg(fit_mesg_defs[FIT_MESG_LAP], &lap_msg);
		lap_msg.timestamp  = p_data->timestamp;
		lap_msg.start_time = start_time;
		lap_msg.total_elapsed_time = (p_data->timestamp - start_time)*1000;
		lap_msg.event = FIT_EVENT_LAP;
		lap_msg.sport = FIT_SPORT_CYCLING;
		WriteMessageDefinition(local_mesg_number, fit_mesg_defs[FIT_MESG_LAP], FIT_LAP_MESG_DEF_SIZE);
		WriteMessage(local_mesg_number, &lap_msg, FIT_LAP_MESG_SIZE);

		// stop marker
		FIT_EVENT_MESG event_msg;
		Fit_InitMesg(fit_mesg_defs[FIT_MESG_EVENT], &event_msg);
		event_msg.timestamp = p_data->timestamp;
		event_msg.event_type = FIT_EVENT_TYPE_STOP_ALL;
		WriteMessageDefinition(local_mesg_number, fit_mesg_defs[FIT_MESG_EVENT], FIT_EVENT_MESG_DEF_SIZE);
		WriteMessage(local_mesg_number, &event_msg, FIT_EVENT_MESG_SIZE);

		// record session
		FIT_SESSION_MESG session_msg;
		Fit_InitMesg(fit_mesg_defs[FIT_MESG_SESSION], &session_msg);
		session_msg.timestamp  = p_data->timestamp;
		session_msg.start_time = start_time;
		session_msg.total_elapsed_time = (p_data->timestamp - start_time)*1000;
		WriteMessageDefinition(local_mesg_number, fit_mesg_defs[FIT_MESG_SESSION], FIT_SESSION_MESG_DEF_SIZE);
		WriteMessage(local_mesg_number, &session_msg, FIT_SESSION_MESG_SIZE);

		// record activity summary
		FIT_ACTIVITY_MESG act_msg;
		Fit_InitMesg(fit_mesg_defs[FIT_MESG_ACTIVITY], &act_msg);
		act_msg.timestamp = p_data->timestamp;
		act_msg.type       = FIT_ACTIVITY_MANUAL;
		act_msg.event_type = FIT_EVENT_TYPE_STOP;
		act_msg.event      = FIT_EVENT_ACTIVITY;
		act_msg.num_sessions = 1;
		WriteMessageDefinition(local_mesg_number, fit_mesg_defs[FIT_MESG_ACTIVITY], FIT_ACTIVITY_MESG_DEF_SIZE);
		WriteMessage(local_mesg_number, &act_msg, FIT_ACTIVITY_MESG_SIZE);

		// Write CRC.
		WriteDataBare(&data_crc, sizeof(FIT_UINT16));

		// Update file header with data size.
		WriteFileHeader();
	}

	return 0;
}
