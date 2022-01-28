/**
 * @file    usbconsole.cpp
 * @brief	USB-over-serial configuration
 *
 * @date Oct 14, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "global.h"

#include "usbconsole.h"
#include "usbcfg.h"

static bool isUsbSerialInitialized = false;

/**
 * start USB serial using hard-coded communications pins (see comments inside the code)
 */
void usb_serial_start() {
	palSetPadMode(EFI_USB_SERIAL_PORT, EFI_USB_SERIAL_PIN_DM, PAL_MODE_ALTERNATE(EFI_USB_AF));
	palSetPadMode(EFI_USB_SERIAL_PORT, EFI_USB_SERIAL_PIN_DP, PAL_MODE_ALTERNATE(EFI_USB_AF));

	/*
	 * Initializes a serial-over-USB CDC driver.
	 */
	sduObjectInit(&EFI_CONSOLE_USB_DEVICE);
	sduStart(&EFI_CONSOLE_USB_DEVICE, &serusbcfg);

	/*
	 * Activates the USB driver and then the USB bus pull-up on D+.
	 * Note, a delay is inserted in order to not have to disconnect the cable
	 * after a reset.
	 */
	usbDisconnectBus(serusbcfg.usbp);
	chThdSleepMilliseconds(250);
	usbStart(serusbcfg.usbp, &usbcfg);
	usbConnectBus(serusbcfg.usbp);

	isUsbSerialInitialized = true;
}

bool is_usb_serial_ready() {
	return isUsbSerialInitialized && EFI_CONSOLE_USB_DEVICE.config->usbp->state == USB_ACTIVE;
}

