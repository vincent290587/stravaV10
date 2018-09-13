/*
 * App.cpp
 *
 *  Created on: 8 oct. 2017
 *      Author: Vincent
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hardfault.h"
#include "ant.h"
#include "i2c.h"
#include "notifications.h"
#include "backlighting.h"
#include "mk64f_parser.h"
#include "helper.h"
#include "bsp.h"
#include "spi.h"
#include "i2c.h"
#include "fec.h"
#include "bsp.h"
#include "nor.h"
#include "app_scheduler.h"
#include "app_timer.h"
#include "nrf_sdm.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_strerror.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_power.h"
#include "nrf_bootloader_info.h"
#include "nrfx_wdt.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "i2c_scheduler.h"
#include "Model.h"
#include "sd_hal.h"
#include "segger_wrapper.h"

#ifdef USB_ENABLED
#include "usb_cdc.h"
#endif

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define DEAD_BEEF           0xDEADBEEF                                  /**< Value used as error code on stack dump. Can be used to identify stack location on stack unwind. */

#define APP_DELAY           APP_TIMER_TICKS(APP_DELAY_MS)

#define SCHED_MAX_EVENT_DATA_SIZE      APP_TIMER_SCHED_EVENT_DATA_SIZE              /**< Maximum size of scheduler events. */
#ifdef SVCALL_AS_NORMAL_FUNCTION
#define SCHED_QUEUE_SIZE               20                                           /**< Maximum number of events in the scheduler queue. More is needed in case of Serialization. */
#else
#define SCHED_QUEUE_SIZE               10                                           /**< Maximum number of events in the scheduler queue. */
#endif

APP_TIMER_DEF(m_job_timer);

nrfx_wdt_channel_id m_channel_id;

extern "C" void ble_ant_init(void);


static volatile bool job_to_do = true;


/**
 * @brief Handler for timer events.
 */
void timer_event_handler(void* p_context)
{
	job_to_do = true;
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

    nor_save_error(id, pc, info);

    switch (id)
    {
#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT
        case NRF_FAULT_ID_SD_ASSERT:
            NRF_LOG_ERROR("SOFTDEVICE: ASSERTION FAILED");
            break;
        case NRF_FAULT_ID_APP_MEMACC:
            NRF_LOG_ERROR("SOFTDEVICE: INVALID MEMORY ACCESS");
            break;
#endif
        case NRF_FAULT_ID_SDK_ASSERT:
        {
            assert_info_t * p_info = (assert_info_t *)info;
            LOG_ERROR("ASSERTION FAILED at %s:%u",
                          p_info->p_file_name,
                          p_info->line_num);
            break;
        }
        case NRF_FAULT_ID_SDK_ERROR:
        {
#if USE_SVIEW
            error_info_t * p_info = (error_info_t *)info;
            SEGGER_SYSVIEW_PrintfHost("ERROR %u at %s:%u",
                          p_info->err_code,
                          p_info->p_file_name,
                          p_info->line_num);
#else
            error_info_t * p_info = (error_info_t *)info;
            LOG_ERROR("ERROR %u [%s] at %s:%u",
                          p_info->err_code,
						  nrf_strerror_get(p_info->err_code),
                          p_info->p_file_name,
                          p_info->line_num);
#endif
            break;
        }
        default:
            NRF_LOG_ERROR("UNKNOWN FAULT at 0x%08X", pc);
            break;
    }

    NRF_LOG_FLUSH();

#ifdef DEBUG_NRF
    NRF_BREAKPOINT_COND;
    // On assert, the system can only recover with a reset.
#endif

}

extern "C" void HardFault_process(HardFault_stack_t * p_stack)
{
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


/**@brief Function for handling bsp events.
 */
static void bsp_evt_handler(bsp_event_t evt)
{

	switch (evt)
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
		nrf_gpio_pin_set(KILL_PIN);
		return true;
	} else if (NRF_PWR_MGMT_EVT_PREPARE_DFU == event) {

#if 0
		// TODO add flag
		ret_code_t err_code = sd_power_gpregret_set(0, BOOTLOADER_DFU_START);
		APP_ERROR_CHECK(err_code);
#endif
		// stop USB
		usb_cdc_close();

		LOG_INFO("Power management allowed to reset to DFU mode.");

		return true;
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
	nrf_gpio_cfg_input(FXOS_INT2, NRF_GPIO_PIN_PULLDOWN);

	nrf_gpio_cfg_output(FXOS_RST);
	nrf_gpio_pin_clear(FXOS_RST);

	// SDC_CS_PIN is configured later
	// LS027_CS_PIN is configured later

	nrf_gpio_cfg_output(BCK_PIN);
	nrf_gpio_pin_clear(BCK_PIN);

	nrf_gpio_cfg_output(SPK_IN);
	nrf_gpio_pin_clear(SPK_IN);

//	nrf_gpio_cfg_output(SCL_PIN_NUMBER);
//	nrf_gpio_pin_set(SCL_PIN_NUMBER);
//
//	nrf_gpio_cfg_output(SDA_PIN_NUMBER);
//	nrf_gpio_pin_set(SDA_PIN_NUMBER);

	// FIX_PIN is configured later

	nrf_gpio_cfg_output(NEO_PIN);
	nrf_gpio_pin_clear(NEO_PIN);

	nrf_gpio_cfg_output(KILL_PIN);
	nrf_gpio_pin_clear(KILL_PIN);
}

void wdt_reload() {
	nrfx_wdt_channel_feed(m_channel_id);
}

static void clock_handler(nrf_drv_clock_evt_type_t event) {

	switch (event) {
	case NRF_DRV_CLOCK_EVT_LFCLK_STARTED:
		LOG_INFO("LFCLK started");
		break;
	case NRF_DRV_CLOCK_EVT_HFCLK_STARTED:
		LOG_INFO("HFCLK started");
		break;
	case NRF_DRV_CLOCK_EVT_CAL_DONE:
		LOG_INFO("CAL done");
		break;
	case NRF_DRV_CLOCK_EVT_CAL_ABORTED:
		LOG_INFO("CAL aborted");
		break;
	}

}

/**
 *
 * @return 0
 */
int main(void)
{
	ret_code_t err_code;

	// errata 20 RTC
//	NRF_CLOCK->EVENTS_LFCLKSTARTED  = 0;
//	NRF_CLOCK->TASKS_LFCLKSTART     = 1;
//	while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0) {}
//	NRF_RTC0->TASKS_STOP = 0;
//	NRF_RTC1->TASKS_STOP = 0;

	// Initialize.
    //Configure WDT.
    nrfx_wdt_config_t wdt_config = NRFX_WDT_DEAFULT_CONFIG;
    err_code = nrfx_wdt_init(&wdt_config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrfx_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);
    nrfx_wdt_enable();

	log_init();

	uint32_t reset_reason = NRF_POWER->RESETREAS;
	NRF_POWER->RESETREAS = 0xffffffff;
	NRF_LOG_WARNING("Reset_reason: 0x%08x.\n", reset_reason);

    char buff[100];
    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "Reset_reason: 0x%04lX",
    		reset_reason);
    vue.addNotif("Event", buff, 4, eNotificationTypeComplete);

	if (reset_reason == 0)
	{
		// watchdog reset
		//while(1) ;
		NVIC_SystemReset();
	}

#ifndef USB_ENABLED
    err_code = nrf_drv_power_init(NULL);
    APP_ERROR_CHECK(err_code);
#endif

	pins_init();

	// clocks init
	err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_handler_item_s clock_h = {
    		.p_next = NULL,
    		.event_handler = clock_handler,
    };

    LOG_INFO("Starting LF clock");
    nrf_drv_clock_lfclk_request(&clock_h);
    while(!nrf_drv_clock_lfclk_is_running())
    {
        /* Just waiting */
    	nrf_delay_ms(1);
    }

    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    nrf_delay_ms(5);
	LOG_INFO("Init start");

	// drivers
	spi_init();
	i2c_init();

	// LCD displayer
	vue.init();

	// diskio + fatfs init
	usb_cdc_diskio_init();

	// SPI flash init
	nor_init();
	nor_read_error();

	LOG_FLUSH();

	// Initialize timer module
#ifdef USB_ENABLED
	// apply correction 0x20 line 97 nrf_drv_usbd_errata.h
	usb_cdc_init();
	usb_cdc_tasks();
#endif

	nrf_pwr_mgmt_init();

#if APP_SCHEDULER_ENABLED
	APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
#endif

	// timers
#ifdef ANT_STACK_SUPPORT_REQD
	ant_timers_init();
#endif

	backlighting_init();

	buttons_leds_init();

	notifications_init(NEO_PIN);

	// init BLE + ANT
#ifdef BLE_STACK_SUPPORT_REQD
	ble_ant_init();
#endif

	err_code = app_timer_create(&m_job_timer, APP_TIMER_MODE_REPEATED, timer_event_handler);
	APP_ERROR_CHECK(err_code);

	err_code = app_timer_start(m_job_timer, APP_DELAY, NULL);
	APP_ERROR_CHECK(err_code);

	sNeopixelOrders neo_order;
	SET_NEO_EVENT_RED(neo_order, eNeoEventNotify, 0);
	notifications_setNotify(&neo_order);


	gps_mgmt.init();

	// init all I2C devices
	i2c_scheduling_init();

	boucle.init();

	NRF_LOG_INFO("App init done");

	for (;;)
	{
		if (job_to_do) {
			// tasks to run non-continuously

			job_to_do = false;

			LOG_INFO("\r\nTask %u", millis());

			wdt_reload();

#ifdef ANT_STACK_SUPPORT_REQD
			roller_manager_tasks();
#endif

#ifdef _DEBUG_TWI
			stc.refresh(nullptr);
			veml.refresh(nullptr);
			fxos_tasks(nullptr);
			ms5637.refresh(nullptr);
#endif

			notifications_tasks();

			backlighting_tasks();

			boucle.tasks();

			if (!millis()) NRF_LOG_WARNING("No millis");
		}

		// tasks
		perform_system_tasks();

	}
}

