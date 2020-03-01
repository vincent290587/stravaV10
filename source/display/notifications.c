#include "stdint.h"
#include <string.h>
#include <stdbool.h>
#include "boards.h"
#include "drv_ws2812.h"
#include "notifications.h"
#include "segger_wrapper.h"


#define ON_STEPS_NB      5
#define ON_TICKS_DEFAULT 3


static neo_sb_init_params_t _params;
static neo_sb_seg_params    m_seg_notif;
static bool                 leds_is_on;     /**< Flag for indicating if LEDs are on. */
static bool                 is_counting_up; /**< Flag for indicating if counter is incrementing or decrementing. */
static int32_t              pause_ticks;
static uint32_t             ratio;

/**
 *
 * @param red
 * @param green
 * @param blue
 * @return
 */
static uint32_t get_notifications_color(uint8_t red, uint8_t green, uint8_t blue ) {

	uint32_t res = (red << 16) | (green << 8) | (blue);
	return res;
}

/**
 *
 * @param red
 * @param green
 * @param blue
 * @return
 */
static void notifications_setColor(uint8_t red, uint8_t green, uint8_t blue) {

	// update neopixel
	uint32_t color = get_notifications_color(red, green, blue);

	drv_ws2812_set_pixel_all(color);
}

/**
 *
 * @param pin_num
 */
void notifications_init(uint8_t pin_num) {

	_params.max = 10;
	_params.min = 0;

	ratio = 0;
	leds_is_on  = false;
	pause_ticks = 0;

	_params.rgb[0] = 0;
	_params.rgb[1] = 0;
	_params.rgb[2] = 0;

	_params.step = _params.max / ON_STEPS_NB;
	_params.on_time_ticks = ON_TICKS_DEFAULT;

	memset(&m_seg_notif, 0, sizeof(m_seg_notif));

	is_counting_up = true;

	drv_ws2812_init(pin_num);
}

/**
 *
 * @param red
 * @param green
 * @param blue
 * @param on_time
 */
static void _set_notify(uint8_t red, uint8_t green, uint8_t blue, uint8_t on_time) {

	_params.rgb[0] = red;
	_params.rgb[1] = green;
	_params.rgb[2] = blue;

	// start process
	is_counting_up = true;
	leds_is_on     = true;
	ratio          = _params.min;
	pause_ticks    = 0;

	_params.on_time_ticks = on_time == 0 ? ON_TICKS_DEFAULT : on_time;
}

/**
 *
 * @param orders
 */
void notifications_setNotify(sNeopixelOrders* orders) {

	switch (orders->event_type) {
	case eNeoEventEmpty:
		break;

	case eNeoEventWeakNotify:
	{
		if (leds_is_on) {

			memset(orders, 0, sizeof(sNeopixelOrders));

			return;
		}
	}
	// no break

	case eNeoEventNotify:
	{
		_set_notify(
				orders->rgb[0],
				orders->rgb[1],
				orders->rgb[2],
				orders->on_time);

		memset(orders, 0, sizeof(sNeopixelOrders));
	}
	break;

	default:
		break;
	}

}

void notifications_segNotify(neo_sb_seg_params* orders) {

	memcpy(&m_seg_notif, orders, sizeof(m_seg_notif));
}

/**
 *
 * @return 1 if no task is active
 */
uint8_t notifications_tasks() {

	if (!leds_is_on) {

		static int on_time = 0;
		static int off_time = 0;

		// handle notifications
		if (m_seg_notif.active) {

			if (on_time == -1) {
				on_time = m_seg_notif.on_time;
				off_time = 0;
			}

			if (on_time) {

				LOG_DEBUG("Light ON");

				on_time--;
				notifications_setColor(m_seg_notif.rgb[0], m_seg_notif.rgb[1], m_seg_notif.rgb[2]);
			} else if (off_time) {

				LOG_DEBUG("Light OFF");

				off_time--;
				notifications_setColor(0, 0, 0);
			} else {

				on_time = m_seg_notif.on_time;
				off_time = m_seg_notif.off_time;
				notifications_setColor(0, 0, 0);
			}

		} else {

			on_time = -1;
			off_time = 0;
			notifications_setColor(0, 0, 0);
		}

	} else {

		// continue process
		if (pause_ticks <= 0)
		{
			if (is_counting_up)
			{
				if ((int)(ratio) >= (int)(_params.max - _params.step))
				{
					// start decrementing.
					is_counting_up = false;
					pause_ticks = _params.on_time_ticks;
				}
				else
				{
					ratio += _params.step;
				}
			}
			else
			{
				if ((int)(ratio) <= (int)(_params.min + _params.step))
				{
					// Min is reached, we are done
					// end process
					leds_is_on = false;
					ratio = 0;
				}
				else
				{
					ratio -= _params.step;
				}
			}
		}
		else
		{
			pause_ticks -= 1;
		}

		notifications_setColor(_params.rgb[0] * ratio * ratio / 255,
		 			_params.rgb[1] * ratio * ratio / 255,
		 			_params.rgb[2] * ratio * ratio / 255);
	}

	drv_ws2812_display(NULL, NULL);

	return 0;
}



