/*
 * lezyne_protocol.h
 *
 *  Created on: 20 mai 2020
 *      Author: vgol
 */

#ifndef LEZYNE_PROTOCOL_H_
#define LEZYNE_PROTOCOL_H_

enum GpsDeviceModel {
	InvalidDevice,
	Y9Mini,
	Y9Power,
	Y9Super,
	Y10Super,
	Y10Macro,
	Y10Micro,
	Y10MicroColor,
	Y10MicroWatch,
	Y10WatchColor,
	Y10Mini,
	Y12Mega,
	Y12MegaColor,
	Y13Super,
	Y13Macro,
};


enum SettingCategory {
	SettingCategoryalert=1,
	SettingCategoryauto,
	SettingCategorypersonal,
	SettingCategorynavigation,
	SettingCategorymode,
	SettingCategoryname,
	SettingCategoryscreen,
	SettingCategorytime,
	SettingCategoryrunningpages,
	SettingCategorybike1,
	SettingCategorybike2,
	SettingCategorybike3,
	SettingCategorybike4,
	SettingCategorybike5,
	SettingCategorystrava,
	SettingCategoryzones,
};


enum IncomingCommands {
	InvalidIncomingCommand = 0,
	FitFileTransferStart = 1,
	FitFileTransferData = 2,
	FitFileTransferEnd = 3,
	SwitchingToHighSpeed = 4,
	ConnectedInHighSpeed = 5,
	SwitchingToLowSpeed = 6,
	ConnectedInLowSpeed = 7,
	DebugText = 8,
	ErrorPacket = 9,
	FileDeleteConfirmation = 10,
	FileListSending = 11,
	NavigationRerouteRequest = 12,
	LiveSensorData = 13,
	LiveSessionStatus = 14,
	GPSReadyToReceiveSegmentList = 15,
	RequestPhoneStatus = 16,
	SegmentFileRequest = 17,
	SegmentFileRequestEnd = 18,
	NavigationNewFileReady = 20,
	SettingsSendStart = 22,
	NavigationGPSCancel = 23,
	SettingsSendData = 24,
	SettingsSendError = 25,
	TrainingSendError = 27,
	MapFileReq = 28,
	IncomingMapFileError = 29,
	RequestMTUUpdate = 30,
	MapFileSizeError = 31,
	MapFileListData = 32,
	MapFileListEnd = 33,
	MapFileDeleteComplete = 34,
	MapFileDeleteError = 35,
	MapFileListError = 36,
	NavigationFailed = 37,
};


enum OutgoingCommands {
	InvalidOutgoigCommand = 0,
	RequestFitFileList = 1,
	RequestFitFileDownload = 2,
	RequestFitFileDelete = 3,
	NewSegmentListReady = 4,
	EmailNotification = 5,
	EmailNotificationBody = 6,
	EmailNotificationNext = 7,
	SMSNotification = 8,
	SMSNotificationBody = 9,
	SMSNotificationNext = 10,
	CallNotification = 11,
	RequestSlowDownloads = 12,
	NavigationNewRoute = 13,
	NavigationRouteDataContinue = 14,
	NavigationStep = 15,
	NavigationStepContinued = 16,
	NavigationRouteData = 17,
	SegmentListItem = 18,
	SegmentListItemDone = 19,
	SegmentFileUploadStart = 20,
	SegmentFileUploadData = 21,
	SegmentFileUploadEnd = 22,
	SettingsRequestV1 = 24,
	PhoneStatus = 25,
	SegmentUpdateCancel = 26,
	NavigationError = 27,
	NavigationNewDestination = 29,
	NavigationNewFile = 30,
	NavigationNewFileDest = 31,
	NavFileUploadDataStart = 32,
	NavFileUploadData = 33,
	NavFileUploadEnd = 34,
	NavigationPhoneCancel_V2 = 35,
	SettingsRequest = 36,
	UpdateSetting = 37,
	UpdateSettingContinue = 38,
	Notification = 39,
	NotificationBody = 40,
	NotificationNext = 41,
	TrainingFileUploadStart = 42,
	TrainingFileUploadData = 43,
	TrainingFileUploadEnd = 44,
	MapFileReadyStart = 45,
	MapFileReadyEnd = 46,
	MapFileStart = 47,
	MapFileData = 48,
	MapFileEnd = 49,
	OutgoingMapFileError = 50,
	MapFileListRequest = 51,
	MapFileDelete = 52,
};


#endif /* LEZYNE_PROTOCOL_H_ */
