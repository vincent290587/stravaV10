/*
 * usb_cdc.h
 *
 *  Created on: 3 juil. 2018
 *      Author: Vincent
 */

#ifndef SOURCE_USB_USB_CDC_H_
#define SOURCE_USB_USB_CDC_H_

#ifdef __cplusplus
extern "C" {
#endif

void usb_cdc_init(void);

void usb_cdc_close(void);

void usb_flush(void);

void usb_cdc_tasks(void);

void usb_print(char c);

void usb_printf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* SOURCE_USB_USB_CDC_H_ */
