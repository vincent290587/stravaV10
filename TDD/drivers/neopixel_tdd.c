/* Lava
 *
 * WS2812B Tricolor LED (neopixel) controller
 *
 *
 * Example code:

	neopixel_strip_t m_strip;
	uint8_t dig_pin_num = 6;
	uint8_t leds_per_strip = 24;
	uint8_t error;
	uint8_t led_to_enable = 10;
	uint8_t red = 255;
	uint8_t green = 0;
	uint8_t blue = 159;

	neopixel_init(&m_strip, dig_pin_num, leds_per_strip);
	neopixel_clear(&m_strip);
	error = neopixel_set_color_and_show(&m_strip, led_to_enable, red, green, blue);
	if (error) {
		//led_to_enable was not within number leds_per_strip
	}
	//clear and remove strip
	neopixel_clear(&m_strip);
	neopixel_destroy(&m_strip);


 * For use with BLE stack, see information below:
	- Include in main.c
		#include "ble_radio_notification.h"
	- Call (see nrf_soc.h: NRF_RADIO_NOTIFICATION_DISTANCES and NRF_APP_PRIORITIES)
		ble_radio_notification_init(NRF_APP_PRIORITY_xxx,
									NRF_RADIO_NOTIFICATION_DISTANCE_xxx,
									your_radio_callback_handler);
	- Create
		void your_radio_callback_handler(bool radio_active)
		{
			if (radio_active == false)
			{
				neopixel_show(&strip1);
				neopixel_show(&strip2);
				//...etc
			}
		}
	- Do not use neopixel_set_color_and_show(...) with BLE, instead use uint8_t neopixel_set_color(...);
 */

#include <stddef.h>
#include "neopixel.h"
#include "segger_wrapper.h"


#define NRF_RADIO_NOTIFICATION_DISTANCE_NEOPIXEL_US

neopixel_strip_t* m_p_strip = NULL;

volatile bool m_neo_orders_ready = false;

////////////       FUNCTIONS

extern void neopixel_update(uint8_t red, uint8_t green, uint8_t blue);

static void _neopixel_show(neopixel_strip_t *strip);

void neopixel_radio_callback_handler(bool radio_active)
{
	if (radio_active == false && m_neo_orders_ready)
	{
		m_neo_orders_ready = false;
		_neopixel_show(m_p_strip);
	}
}

void neopixel_init(neopixel_strip_t *strip, uint8_t pin_num, uint16_t num_leds)
{
	strip->pin_num = pin_num;
	strip->num_leds = num_leds;

	for (int i = 0; i < num_leds; i++)
	{
		strip->leds[i].simple.g = 0;
		strip->leds[i].simple.r = 0;
		strip->leds[i].simple.b = 0;
	}

}

void neopixel_clear(neopixel_strip_t *strip)
{
	for (int i = 0; i < strip->num_leds; i++)
	{
		strip->leds[i].simple.g = 0;
		strip->leds[i].simple.r = 0;
		strip->leds[i].simple.b = 0;
	}
	neopixel_show(strip);
}


static void _neopixel_show(neopixel_strip_t *strip)
{
	neopixel_update(strip->leds[0].simple.r, strip->leds[0].simple.g, strip->leds[0].simple.b);
}

void neopixel_show(neopixel_strip_t *strip)
{
	m_neo_orders_ready = true;
	m_p_strip = strip;

	LOG_INFO("NeoPixel updated");
}

uint8_t neopixel_set_color(neopixel_strip_t *strip, uint16_t index, uint8_t red, uint8_t green, uint8_t blue )
{
	if (index < strip->num_leds)
	{
		strip->leds[index].simple.r = red;
		strip->leds[index].simple.g = green;
		strip->leds[index].simple.b = blue;
	} else {
		return 1;
	}
	return 0;
}

uint8_t neopixel_set_color_and_show(neopixel_strip_t *strip, uint16_t index, uint8_t red, uint8_t green, uint8_t blue)
{
	if (index < strip->num_leds)
	{
		strip->leds[index].simple.r = red;
		strip->leds[index].simple.g = green;
		strip->leds[index].simple.b = blue;
		neopixel_show(strip);
	} else {
		return 1;
	}

	return 0;
}


