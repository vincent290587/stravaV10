/*
 * segger_wrapper.c
 *
 *  Created on: 5 oct. 2017
 *      Author: Vincent
 */

#include "Attitude.h"
#include "segger_wrapper.h"
#include "task_manager_wrapper.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern sAppErrorDescr m_app_error;

#if USE_SVIEW

static SEGGER_SYSVIEW_MODULE m_module = {
		"M=stravaV10",
		30,
		0,
};

#endif

/*******************************************************************************
 * Functions
 ******************************************************************************/

static uint32_t m_cur_void_id;

void sysview_task_void_enter(uint32_t void_id) {
	m_cur_void_id = void_id;
	m_app_error.void_id = void_id;
	W_SYSVIEW_RecordVoid(m_cur_void_id);
}

void sysview_task_u32_enter(uint32_t void_id, uint32_t data) {
	m_cur_void_id = void_id;
	m_app_error.void_id = void_id;
	W_SYSVIEW_RecordU32(m_cur_void_id, data);
}

void sysview_task_void_exit(uint32_t void_id) {
	W_SYSVIEW_RecordEndCall(void_id);
	m_cur_void_id = 0;
	m_app_error.void_id = 0;
}

/*********************************************************************
*
*       segger_init()
*
*  Function description
*    Record task information.
*/
void segger_init(void) {

//#if USE_RTT && !USE_SVIEW
//	  // RTT
//	  SEGGER_RTT_Init();
//#endif

#if USE_SVIEW

#warning "SysView is active"

	  SEGGER_SYSVIEW_Conf();

	  SEGGER_SYSVIEW_Start();

	  SEGGER_SYSVIEW_RegisterModule(&m_module);

#endif

}

/*********************************************************************
*
*       SYSVIEW_SendTaskInfo()
*
*  Function description
*    Record task information.
*/
void segger_sendTaskInfo(U32 TaskID, const char* sName, unsigned Prio, U32 StackBase, unsigned StackSize) {
#if USE_SVIEW
  SEGGER_SYSVIEW_TASKINFO TaskInfo;

  memset(&TaskInfo, 0, sizeof(TaskInfo)); // Fill all elements with 0 to allow extending the structure in future version without breaking the code
  TaskInfo.TaskID     = TaskID;
  TaskInfo.sName      = sName;
  TaskInfo.Prio       = Prio;
  TaskInfo.StackBase  = StackBase;
  TaskInfo.StackSize  = StackSize;

  SEGGER_SYSVIEW_SendTaskInfo(&TaskInfo);
#endif
}
