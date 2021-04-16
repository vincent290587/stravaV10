/*
 * App.cpp
 *
 *  Created on: 8 oct. 2017
 *      Author: Vincent
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "boards.h"
#include "hardfault.h"
#include "i2c.h"
#include "notifications.h"
#include "backlighting.h"
#include "mk64f_parser.h"
#include "helper.h"
#include "bsp.h"
#include "spi.h"
#include "i2c.h"
#include "fram.h"
#include "app_scheduler.h"
#include "app_timer.h"
#include "nrf_sdm.h"
#include "nrf_strerror.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_clock.h"
#include "power_scheduler.h"
#include "task_manager_wrapper.h"
#include "nrf_bootloader_info.h"
#include "nrfx_wdt.h"
#include "nrf_gpio.h"
#include "diskio_sdc.h"
#include "diskio_nor.h"
#include "nrf_delay.h"
#include "i2c_scheduler.h"
#include "ant_device_manager.h"
#include "Model.h"
#include "sd_hal.h"
#include "ble_api_base.h"
#include "segger_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "nrf_pwr_mgmt.h"
#ifdef __cplusplus
}
#endif

#ifdef USB_ENABLED
#include "usb_cdc.h"
#endif

#define APP_DELAY           APP_TIMER_TICKS(APP_TIMEOUT_DELAY_MS)

#define SCHED_MAX_EVENT_DATA_SIZE      APP_TIMER_SCHED_EVENT_DATA_SIZE              /**< Maximum size of scheduler events. */
#ifdef SVCALL_AS_NORMAL_FUNCTION
#define SCHED_QUEUE_SIZE               20                                           /**< Maximum number of events in the scheduler queue. More is needed in case of Serialization. */
#else
#define SCHED_QUEUE_SIZE               20                                           /**< Maximum number of events in the scheduler queue. */
#endif

APP_TIMER_DEF(m_job_timer);

nrfx_wdt_channel_id m_channel_id;

static bsp_event_t m_bsp_evt = BSP_EVENT_NOTHING;


/**
 * @brief Handler for timer events.
 */
void timer_event_handler(void* p_context)
{
	W_SYSVIEW_RecordEnterISR();

	if (task_manager_is_started()) {
		task_tick_manage(APP_TIMEOUT_DELAY_MS);
	}

	W_SYSVIEW_RecordExitISR();
}


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
extern "C" void assert_nrf_callback(uint16_t line_num, const uint8_t * file_name)
{
    assert_info_t assert_info =
    {
        .line_num    = line_num,
        .p_file_name = file_name,
    };
    app_error_fault_handler(NRF_FAULT_ID_SDK_ASSERT, 0, (uint32_t)(&assert_info));

#ifndef DEBUG_NRF_USER
    LOG_WARNING("System reset");
    LOG_FLUSH();
    NVIC_SystemReset();
#else
    NRF_BREAKPOINT_COND;

    bool loop = true;
    while (loop) ;
#endif // DEBUG

    UNUSED_VARIABLE(assert_info);
}

/**
 *
 * @param id
 * @param pc
 * @param info
 */
extern "C" void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    NRF_LOG_FLUSH();

    m_app_error.err_desc.id = id;
    m_app_error.err_desc.pc = pc;
	m_app_error.err_desc.crc = SYSTEM_DESCR_POS_CRC;

	if (!info) {
		snprintf(m_app_error.err_desc._buffer, sizeof(m_app_error.err_desc._buffer),
				"info arg is 0 id %lu pc %lu",
				id, pc);
#if USE_SVIEW
		SEGGER_SYSVIEW_Error(m_app_error.err_desc._buffer);
#else
		LOG_ERROR(m_app_error.err_desc._buffer);
		LOG_INFO(m_app_error.err_desc._buffer);
#endif
		return;
	}

    switch (id)
    {
#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT
        case NRF_FAULT_ID_SD_ASSERT:
        	LOG_ERROR("SOFTDEVICE: ASSERTION FAILED");
            break;
        case NRF_FAULT_ID_APP_MEMACC:
        	LOG_ERROR("SOFTDEVICE: INVALID MEMORY ACCESS");
            break;
#endif
        case NRF_FAULT_ID_SDK_ASSERT:
        {
            assert_info_t * p_info = (assert_info_t *)info;
            snprintf(m_app_error.err_desc._buffer, sizeof(m_app_error.err_desc._buffer),
        			"ASSERTION FAILED at %s:%u",
                    p_info->p_file_name,
                    p_info->line_num);
            break;
        }
        case NRF_FAULT_ID_SDK_ERROR:
        {
        	error_info_t * p_info = (error_info_t *)info;
        	snprintf(m_app_error.err_desc._buffer, sizeof(m_app_error.err_desc._buffer),
        			"ERROR %lu [%s] at %s:%lu",
					p_info->err_code,
					nrf_strerror_get(p_info->err_code),
					p_info->p_file_name,
					p_info->line_num);
            break;
        }
        default:
            NRF_LOG_ERROR("UNKNOWN FAULT at 0x%08X", pc);
        	snprintf(m_app_error.err_desc._buffer, sizeof(m_app_error.err_desc._buffer),
        			"UNKNOWN FAULT at 0x%08lX", pc);
            break;
    }

#if USE_SVIEW
            SEGGER_SYSVIEW_Error(m_app_error.err_desc._buffer);
#else
            LOG_ERROR(m_app_error.err_desc._buffer);
            LOG_INFO(m_app_error.err_desc._buffer);
#endif

    NRF_LOG_FLUSH();

#ifdef DEBUG_NRF
    if (id != NRF_FAULT_ID_SDK_ERROR) NRF_BREAKPOINT_COND;
    // On assert, the system can only recover with a reset.
#endif

}

extern "C" void HardFault_process(HardFault_stack_t * p_stack)
{
	LOG_ERROR("HardFault: pc=%u lr=%u psr=%u", p_stack->pc, p_stack->lr, p_stack->psr);

	m_app_error.hf_desc.crc = SYSTEM_DESCR_POS_CRC;
	memcpy(&m_app_error.hf_desc.stck, p_stack, sizeof(HardFault_stack_t));

#ifdef DEBUG_NRF
    NRF_BREAKPOINT_COND;
    // On hardfault, the system can only recover with a reset.

    bool loop = true;
    while (loop) ;
#endif
    // Restart the system by default
    NVIC_SystemReset();
}

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
	ret_code_t err_code = NRF_LOG_INIT(millis);
	APP_ERROR_CHECK(err_code);

	NRF_LOG_DEFAULT_BACKENDS_INIT();

	SVIEW_INIT();

}

/**@brief Interrupt function for handling bsp events.
 */
static void bsp_evt_handler(bsp_event_t evt)
{
	m_bsp_evt = evt;

	// unblock periph servicing task
	if (m_tasks_id.peripherals_id != TASK_ID_INVALID) {
		w_task_delay_cancel(m_tasks_id.peripherals_id);
	}
}

/**@brief Function for handling bsp events.
 */
void bsp_tasks(void)
{
	switch (m_bsp_evt)
	{
	case BSP_EVENT_KEY_0:
		vue.tasks(eButtonsEventLeft);
		break;
	case BSP_EVENT_KEY_1:
		vue.tasks(eButtonsEventCenter);
		break;
	case BSP_EVENT_KEY_2:
		vue.tasks(eButtonsEventRight);
		break;
	default:
		return; // no implementation needed
	}

	// clear event
	m_bsp_evt = BSP_EVENT_NOTHING;
}

/**@brief Handler for shutdown preparation.
 *
 * @details During shutdown procedures, this function will be called at a 1 second interval
 *          untill the function returns true. When the function returns true, it means that the
 *          app is ready to reset to DFU mode.
 *
 * @param[in]   event   Power manager event.
 *
 * @retval  True if shutdown is allowed by this power manager handler, otherwise false.
 */
static bool app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
	if (NRF_PWR_MGMT_EVT_PREPARE_SYSOFF == event) {

		power_scheduler__shutdown();
#if defined (PROTO_V11)
		return false;
#endif

	} else if (NRF_PWR_MGMT_EVT_PREPARE_DFU == event) {

#if 0
		// TODO add flag
		ret_code_t err_code = sd_power_gpregret_set(0, BOOTLOADER_DFU_START);
		APP_ERROR_CHECK(err_code);
#endif

#ifdef USB_ENABLED
		// stop USB
		usb_cdc_close();
#endif

		LOG_INFO("Power management allowed to reset to DFU mode.");

	}

	return true;
}

NRF_PWR_MGMT_HANDLER_REGISTER(app_shutdown_handler, 0);


/**
 * @brief WDT events handler.
 */
void wdt_event_handler(void)
{
    //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
}


/**@brief Function for initializing buttons and LEDs.
 *
 * @param[out] p_erase_bonds  True if the clear bonds button was pressed to wake the application up.
 */
static void buttons_leds_init(void)
{
	uint32_t err_code = bsp_init(BSP_INIT_BUTTONS, bsp_evt_handler);
	APP_ERROR_CHECK(err_code);
}

static void pins_init(void)
{
	nrf_gpio_cfg_input(FXOS_INT1, NRF_GPIO_PIN_PULLDOWN);

	nrf_gpio_cfg_output(FXOS_RST);
	nrf_gpio_pin_clear(FXOS_RST);

	// LS027_CS_PIN is configured later

#ifdef BCK_PIN
	nrf_gpio_cfg_output(BCK_PIN);
	nrf_gpio_pin_clear(BCK_PIN);
#endif

	// FIX_PIN is configured later

	nrf_gpio_cfg_output(NEO_PIN);
	nrf_gpio_pin_clear(NEO_PIN);

#ifdef KILL_PIN
	nrf_gpio_cfg_output(KILL_PIN);
	nrf_gpio_pin_clear(KILL_PIN);
#endif

}

void wdt_reload() {
	nrfx_wdt_channel_feed(m_channel_id);
}


#ifdef SOFTDEVICE_PRESENT
/**@brief Function for initializing the softdevice
 *
 * @details Initializes the SoftDevice
 */
#include "nrf_sdh.h"
static void sdh_init(void)
{
	ret_code_t err_code;

	err_code = nrf_sdh_enable_request();
	APP_ERROR_CHECK(err_code);

	ASSERT(nrf_sdh_is_enabled());

}
#endif


/**
 *
 * @return 0
 */
int main(void)
{
	ret_code_t err_code;

	m_tasks_id.boucle_id = TASK_ID_INVALID;
	m_tasks_id.peripherals_id = TASK_ID_INVALID;
	m_tasks_id.ls027_id = TASK_ID_INVALID;
	m_tasks_id.uart_id  = TASK_ID_INVALID;
	m_tasks_id.usb_id   = TASK_ID_INVALID;

#if CONFIG_JLINK_MONITOR_ENABLED
	NVIC_SetPriority(DebugMonitor_IRQn, _PRIO_SD_LOW);
#warning "!! MONITORING active !!"
#endif

	// Initialize.
    //Configure WDT.
    nrfx_wdt_config_t wdt_config = NRFX_WDT_DEAFULT_CONFIG;
    err_code = nrfx_wdt_init(&wdt_config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrfx_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);
    nrfx_wdt_enable();

	log_init();

#ifdef FPU_INTERRUPT_MODE
    // Enable FPU interrupt
    NVIC_SetPriority(FPU_IRQn, APP_IRQ_PRIORITY_LOWEST);
    NVIC_ClearPendingIRQ(FPU_IRQn);
    NVIC_EnableIRQ(FPU_IRQn);
#endif

	pins_init();

#if APP_SCHEDULER_ENABLED
	APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
#endif

    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

	LOG_WARNING("Init start");

	// check for errors
	if (m_app_error.hf_desc.crc == SYSTEM_DESCR_POS_CRC) {
		LOG_ERROR("Hard Fault found");
		String message = "Hardfault happened: pc = 0x";
		message += String(m_app_error.hf_desc.stck.pc, HEX);
		message += " lr = 0x";
		message += String(m_app_error.hf_desc.stck.lr, HEX);
		message += " psr = 0x";
		message += String(m_app_error.hf_desc.stck.psr, HEX);
		message += " in void ";
		message += m_app_error.void_id;
		LOG_ERROR(message.c_str());
	    vue.addNotif("Error", message.c_str(), 8, eNotificationTypeComplete);
		memset(&m_app_error.hf_desc, 0, sizeof(m_app_error.hf_desc));
	}
	if (m_app_error.err_desc.crc == SYSTEM_DESCR_POS_CRC) {

		LOG_ERROR("Also contains error...");
	}

	// drivers
	spi_init();
	i2c_init();

	delay_ms(10);

	// diskio + fatfs init
#if defined( USE_MEMORY_NOR )
	diskio_nor_init();
#endif
#if defined( USE_MEMORY_SDC )
	diskio_sdc_init();
#endif

	// LCD displayer
	vue.init();

	// Initialize timer module
#ifdef USB_ENABLED
	usb_cdc_init();
#endif

	nrf_pwr_mgmt_init();

	backlighting_init();

	buttons_leds_init();

	notifications_init(NEO_PIN);

	wdt_reload();

	// init BLE + ANT
#ifdef SOFTDEVICE_PRESENT
	sdh_init();
#endif
#if defined (FDS_PRESENT) || defined (FRAM_PRESENT)
	fram_init_sensor();
	u_settings.enforceConfigVersion();
#endif
#if defined (BLE_STACK_SUPPORT_REQD)
	ble_init();
#endif
#if defined (ANT_STACK_SUPPORT_REQD)
	ant_device_manager_init();
#endif

	err_code = app_timer_create(&m_job_timer, APP_TIMER_MODE_REPEATED, timer_event_handler);
	APP_ERROR_CHECK(err_code);

	err_code = app_timer_start(m_job_timer, APP_DELAY, &m_tasks_id);
	APP_ERROR_CHECK(err_code);

	sNeopixelOrders neo_order;
	SET_NEO_EVENT_RED(neo_order, eNeoEventNotify, 0);
	notifications_setNotify(&neo_order);

	gps_mgmt.init();

	// init all I2C devices
	i2c_scheduling_init();

	LOG_INFO("App init done");

#if APP_SCHEDULER_ENABLED
		app_sched_execute();
#endif

	m_tasks_id.boucle_id      = task_create	(boucle_task, "boucle_tasks", NULL);
	m_tasks_id.peripherals_id = task_create	(peripherals_task, "peripherals_task", NULL);
	m_tasks_id.ls027_id       = task_create	(ls027_task, "ls027_task", NULL);

	// does not return
	task_manager_start(idle_task, &m_tasks_id);

}

