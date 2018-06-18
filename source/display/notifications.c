#include <string.h>
#include <stdbool.h>
#include "notifications.h"


#define ON_STEPS_NB      5
#define ON_TICKS_DEFAULT 3


static neopixel_strip_t     strip;
static neo_sb_init_params_t _params;
static bool                 leds_is_on;     /**< Flag for indicating if LEDs are on. */
static bool                 is_counting_up; /**< Flag for indicating if counter is incrementing or decrementing. */
static int32_t              pause_ticks;
static uint32_t             ratio;

/**
 *
 */
static void notifications_clear() {
	neopixel_clear(&strip);
}

/**
 *
 */
static void notifications_show() {
	neopixel_show(&strip);
}

/**
 *
 * @param red
 * @param green
 * @param blue
 * @return
 */
static uint8_t notifications_setColor(uint8_t red, uint8_t green, uint8_t blue ) {
	return neopixel_set_color(&strip, 0, red, green, blue );
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

	is_counting_up = true;

	neopixel_init(&strip, pin_num, 1);

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
		// no break
	}

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

/**
 *
 * @return 1 if no task is active
 */
uint8_t notifications_tasks() {

	if (!leds_is_on) {
		notifications_clear();
		return 1;
	}

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
				notifications_clear();
				return 1;
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

	// update neopixel
	notifications_setColor(_params.rgb[0] * ratio * ratio / 255,
			_params.rgb[1] * ratio * ratio / 255,
			_params.rgb[2] * ratio * ratio / 255);
	notifications_show();

	return 0;
}



