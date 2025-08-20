🔴 RED ALERT 🔴 STATUS UNKNOWN 🔴 Most usages are either F042 or lambda-x2, this wideband-F103 design is a bit abandoned we never got perfect HW+FW match 🔴

Self-contained LSU wideband supporting both STM32F103 and GD32F103

See https://github.com/mck1117/wideband

115200 baud

[.ini](https://github.com/dron0gus/wideband/blob/master/firmware/ini/rusefi_wb_f1.ini)


# Changelog


# rev 4

* Iteration of rev 3 with fixed PCB dimensions
* Improved RC filter on digital input lines
* Pinout changes https://github.com/rusefi/rusefi-hardware/pull/153

# rev 3

* New board
* Microcontroller replaced
* Added four digital inputs
* Added bluetooth module
* Now LQFP-64
* In-action demo https://youtu.be/ZzZXdSdfkbs

# rev 2

* Firmware binaries https://github.com/rusefi/rusefi-hardware/releases/tag/20220904
* Silkscreen improvements on back side
* 2x7 debug port moved to top side
* VBatt sense added
* Still LQFP-48

# rev 1

* Works with LQFP-48 STM32 and GD32
* PWM required HW hacks
