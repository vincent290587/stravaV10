/*
 * App.cpp
 *
 *  Created on: 8 oct. 2017
 *      Author: Vincent
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
#include "bsp_btn_ble.h"
#include "app_scheduler.h"
#include "app_timer.h"
#include "nrf_sdm.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_wdt.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "i2c_scheduler.h"
#include "Model.h"
#include "sd_hal.h"
#include "segger_wrapper.h"

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

nrf_drv_wdt_channel_id m_channel_id;

extern "C" void ble_ant_init(void);


static volatile bool job_to_do = false;


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
void assert_nrf_callback(uint16_t line_num, const uint8_t * file_name)
{
    assert_info_t assert_info =
    {
        .line_num    = line_num,
        .p_file_name = file_name,
    };
    app_error_fault_handler(NRF_FAULT_ID_SDK_ASSERT, 0, (uint32_t)(&assert_info));

#ifndef DEBUG_NRF
    NRF_LOG_WARNING("System reset");
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
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    NRF_LOG_FLUSH();

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
            NRF_LOG_ERROR("ASSERTION FAILED at %s:%u",
                          p_info->p_file_name,
                          p_info->line_num);
            break;
        }
        case NRF_FAULT_ID_SDK_ERROR:
        {
            error_info_t * p_info = (error_info_t *)info;
#ifndef USE_SVIEW
            NRF_LOG_ERROR("ERROR %u [%s] at %s:%u",
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

#ifdef DEBUG_NRF
    NRF_BREAKPOINT_COND;
    // On assert, the system can only recover with a reset.
#endif

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
	}

	NRF_LOG_INFO("Power management allowed to reset to DFU mode.");
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

	// PPS_PIN is configured later
	// FIX_PIN is configured later

	nrf_gpio_cfg_output(NEO_PIN);
	nrf_gpio_pin_clear(NEO_PIN);
	nrf_gpio_cfg_output(KILL_PIN);
	nrf_gpio_pin_clear(KILL_PIN);
}

/**
 *
 * @return 0
 */
int main(void)
{
	ret_code_t err_code;

	// TODO check nrf_power.h when S340 is ready

	pins_init();

	// Initialize.
    //Configure WDT.
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);
    nrf_drv_wdt_enable();

	log_init();

	NRF_LOG_INFO("Init start");

	nrf_pwr_mgmt_init();

	APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);

	// Initialize timer module
	millis_init();

	nrf_delay_ms(2000);

	// drivers
	spi_init();
	i2c_init();
	//// uart is started later

	// SD functions
	nrf_drv_wdt_channel_feed(m_channel_id);
	sd_functions_init();

	ant_timers_init();

	backlighting_init();

	buttons_leds_init();

	notifications_init(NEO_PIN);

	// init BLE + ANT
	ble_ant_init();

	err_code = app_timer_create(&m_job_timer, APP_TIMER_MODE_REPEATED, timer_event_handler);
	APP_ERROR_CHECK(err_code);

	err_code = app_timer_start(m_job_timer, APP_DELAY, NULL);
	APP_ERROR_CHECK(err_code);

	sNeopixelOrders neo_order;
	SET_NEO_EVENT_RED(neo_order, eNeoEventNotify, 0);
	notifications_setNotify(&neo_order);

	// LCD displayer
	vue.init();

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

			NRF_LOG_INFO("Job");

			roller_manager_tasks();

#ifdef _DEBUG_TWI
//			ms5637.computeTempAndPressure(0,0);
#endif

//			nrf_gpio_pin_toggle(LED_PIN);

			notifications_tasks();

			backlighting_tasks();

			boucle.tasks();

			nrf_drv_wdt_channel_feed(m_channel_id);

		}

		// tasks
		perform_system_tasks();

		app_sched_execute();

		if (NRF_LOG_PROCESS() == false)
		{
			nrf_pwr_mgmt_run();
		}

	}
}

