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

// once a put times out we keep dropping without waiting until the host drains the queue again,
// so an enumerated-but-not-open port costs one timeout, not one per character
static bool isDroppingOutput = false;

static msg_t console_put(void *instance, uint8_t b) {
	(void)instance;
	if (!is_usb_serial_ready()) {
		return MSG_OK;
	}
	sysinterval_t timeout = isDroppingOutput ? TIME_IMMEDIATE : TIME_MS2I(100);
	msg_t result = chnPutTimeout(&EFI_CONSOLE_USB_DEVICE, b, timeout);
	isDroppingOutput = result != MSG_OK;
	return MSG_OK;
}

static size_t console_write(void *instance, const uint8_t *bp, size_t n) {
	for (size_t i = 0; i < n; i++) {
		console_put(instance, bp[i]);
	}
	return n;
}

static size_t console_read(void *instance, uint8_t *bp, size_t n) {
	(void)instance;
	(void)bp;
	(void)n;
	return 0;
}

static msg_t console_get(void *instance) {
	(void)instance;
	return MSG_RESET;
}

static const struct BaseSequentialStreamVMT nonBlockingConsoleVmt = {
	(size_t)0, console_write, console_read, console_put, console_get
};

static BaseSequentialStream nonBlockingConsole = { &nonBlockingConsoleVmt };

BaseSequentialStream *getNonBlockingConsole() {
	return &nonBlockingConsole;
}

