#pragma once

#define EFI_USB_AF 10U
#define EFI_USB_SERIAL_PORT GPIOA
#define EFI_USB_SERIAL_PIN_DM 11
#define EFI_USB_SERIAL_PIN_DP 12

#define EFI_CONSOLE_USB_DEVICE SDU1

// we are lucky - all CAN pins use the same AF
#define EFI_CAN_AF 9

#define CAN_PORT GPIOD
#define CAN_PIN_RX 0
#define CAN_PIN_TX 1

struct io_pin {
	ioportid_t port;
	ioportmask_t pin;
};
