/*
 * Copyright (c) 2016-2025  Moddable Tech, Inc.
 *
 *   This file is part of the Moddable SDK Runtime.
 * 
 *   The Moddable SDK Runtime is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 * 
 *   The Moddable SDK Runtime is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with the Moddable SDK Runtime.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define __XS6PLATFORMMINIMAL__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "driver/usb_serial_jtag.h"

#include "modInstrumentation.h"

#include "xs.h"
#include "xsHost.h"
#include "xsHosts.h"

#include "mc.defines.h"

#if MODDEF_ECMA419_ENABLED
	#include "common/builtinCommon.h"
#endif

#ifndef XT_STACK_EXTRA
	#define XT_STACK_EXTRA  512
	#define XT_STACK_EXTRA_CLIB 1024
#endif

//#define WEAK __attribute__((weak))
#define WEAK

#ifndef DEBUGGER_SPEED
	#define DEBUGGER_SPEED 921600
#endif

static xsMachine *gThe;		// copied in from main

extern void fx_putc(void *refcon, char c);		//@@

/*
	xsbug IP address

	IP address either:
		0,0,0,0 - no xsbug connection
		127,0,0,7 - xsbug over serial
		w,x,y,z - xsbug over TCP (address of computer running xsbug)
*/

#define XSDEBUG_NONE 0,0,0,0
#define XSDEBUG_SERIAL 127,0,0,7
#ifndef DEBUG_IP
	#define DEBUG_IP XSDEBUG_SERIAL
#endif

#ifdef mxDebug
	WEAK unsigned char gXSBUG[4] = {DEBUG_IP};
#endif

#ifdef mxDebug
uint8_t jtagReady = 1;
uint8_t jtag0_position = 0, jtag0_available = 0;
uint8_t jtag1_position = 0, jtag1_available = 0;
static uint8_t jtag0[128];
static uint8_t jtag1[128];

static void debug_task(void *pvParameter)
{
	usb_serial_jtag_driver_config_t cfg = { .rx_buffer_size = 512, .tx_buffer_size = 512 };
	usb_serial_jtag_driver_install(&cfg);

	while (true) {

		if (0 == jtagReady) {
			int amt = usb_serial_jtag_read_bytes(jtag1, sizeof(jtag1), 1);
			if (0 == amt)
				continue;
			jtag1_position = 0;
			jtag1_available = (uint8_t)amt;  
		}
		else {
			int amt = usb_serial_jtag_read_bytes(jtag0, sizeof(jtag0), 1);
			if (0 == amt)
				continue;
			jtag0_position = 0;
			jtag0_available = (uint8_t)amt;  
		}

		fxReceiveLoop();
	}
}
#endif

/*
	Required functions provided by application
	to enable serial port for diagnostic information and debugging
*/

WEAK void modLog_transmit(const char *msg)
{
	uint8_t c;

#ifdef mxDebug
	if (gThe) {
		while (0 != (c = c_read8(msg++)))
			fx_putc(gThe, c);
		fx_putc(gThe, 0);
	}
	else
#endif
	{
		while (0 != (c = c_read8(msg++)))
			ESP_putc(c);
		ESP_putc(13);
		ESP_putc(10);
	}
}

WEAK void ESP_put(uint8_t *c, int count) {
	int sent = 0;
	while (count > 0) {
		sent = usb_serial_jtag_write_bytes(c, count, 10);
		if (sent <= 0)
			break;
		c += sent;
		count -= sent;
	}
}

WEAK void ESP_putc(int c) {
	char cx = c;
	usb_serial_jtag_write_bytes(&cx, 1, 1);
}

WEAK int ESP_getc(void) {
#ifdef mxDebug
	if (0 == jtagReady) {
		if (jtag0_available) {
			jtag0_available--;
			return jtag0[jtag0_position++];
		}
		else if (jtag1_available) {
			jtagReady = 1;
			jtag1_available--;
			return jtag1[jtag1_position++];
		}
	}
	else {
		if (jtag1_available) {
			jtag1_available--;
			return jtag1[jtag1_position++];
		}
		else if (jtag0_available) {
			jtagReady = 0;
			jtag0_available--;
			return jtag0[jtag0_position++];
		}
	}
#endif /* xDebug */
	return -1;
}

WEAK uint8_t ESP_isReadable() {
#ifdef mxDebug
	return jtag0_available || jtag1_available;
#else
	return 0;
#endif
}

WEAK uint8_t ESP_setBaud(int baud) {
	return 1;
}

void setupDebugger(xsMachine *the) {
	gThe = the;

#ifdef mxDebug
    xTaskCreate(debug_task, "debug", (768 + XT_STACK_EXTRA) / sizeof(StackType_t), 0, 8, NULL);
    printf("USB CONNECTED\r\n");
#endif
}

