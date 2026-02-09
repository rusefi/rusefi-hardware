# About

🟢 rev1 is known to work 🟢

pinout source https://github.com/rusefi/rusefi/tree/master/firmware/config/boards/lambda-x2

https://www.shop.rusefi.com/shop/p/dual-channel-wbo

![Uploading Screenshot from 2022-04-24 12-51-07.png…]()

Features:
* STM32F103 dual channel wideband AFR controller supporting Bosch LSU4.9 and LSU4.2 (LSU_ADV work in progress)
* CAN and analog output
* Bluetooth or USRT TunerStudio connectivity

Options (not icluded in default BOM)
* x2 EGT input using MAX31855KASA or MAX31856
* x2 auxilary analog input: 0..5V with pull-up or pull-down (configurable by soldering resistor). One can be used to source 5V to external sensor.
* x2 2 auxilary output 0..5V. Also can be used for slow PWM signal output.
* x2 open drain outputs

In development:
* MegaSquirt compatible pass-through connection over CAN http://www.msgpio.com/manuals/mshift/cpt.html (in progress)

See [interactive pinout](https://rusefi.com/docs/pinouts/lambda-x2/)

See [wire colors](https://github.com/rusefi/rusefi/wiki/WBO#naming-convention) for standart LSU wire collors and connectors pinouts.

Current stable [FW release](https://github.com/dron0gus/wideband/releases/tag/release_zero)

Firmware [sources](https://github.com/dron0gus/wideband) (based on https://github.com/mck1117/wideband).

See rusEFI forum: [F103 dual channel wideband controller + EGT + 2 x AUX In + 2 Aux out](https://rusefi.com/forum/viewtopic.php?f=4&t=2314)

See also https://github.com/rusefi/rusefi/wiki/WBO

# Known issues

## DFU mode/floating PB2

BOOT0 = 1, BOOT1 = 0 pattern is used to enter DFU bootloader on power/reset. BOOT0 is controled by jumper/button. BOOT1 (PB2) is used as bias current source for LSU4.9 and floating.

Use jumper wire to connect PB2 (through bias resistor) to GND: connect A2 pin of main connector with GND (J4 AUX connector pin 10)

![20230930_110548](https://github.com/rusefi/rusefi-hardware/assets/28624689/433ceeff-4cae-434b-8f47-b92947faede2)

## ESR measurement crosstalk between channels (rev0 and rev1)

HW has three different outputs for Nernst cell ESR measurement with different output resistance: 6.8K for LSU4.2, 22K for LSU4.9 and 47K for LSUADV. Idea was to allow the user to set the type of sensor from SW only.

Each ESR output is driven by one MCU GPIO. GPIOs for unused ESR measurement outputs are kept in Hi-Z (input) state and does not affect ESR measurement.

Same GPIO drivers ESR outputs for both channels. **This is the root cause of problem**

Problem: unused ESR output with GPIO in Hi-Z state connect Nernst cell outputs from left and right channel through two resistors. This causes crosstalk between channels and inaccurate ESR measurement. Wrong ESR measurement causes incorrect sensor temperature calculation. This can lead to inaccurate AFR measurement, overheating or underheating of the sensor.

Same issue for LSU4.9 bias resistors R22 and R70.

Solution:

Boards come with all ESR drive resistors populated. You have to remove unnecessary resistors.

If LSU4.2 sensor is used: remove R24, R25, R71, R72, R22, R70. Keep R41 and R73 - 6.8K

If LSU4.9 sensor is used: remove R25, R41, R72, R73. Keep R24 and R71 - 22K

If LSUADV sensor is used: remove R24, R41, R71, R73, R22, R70. Keep R25 and R72 - 47K

This will be (or not) solved in next revision of HW.


# Pinout

## Connectors pinout

[Main connector interactive Pinout](https://rusefi.com/docs/pinouts/lambda-x2/)

J4 is AUX connector available on PCB. SPI and I2C interfaces along with 3.3V and 5V are routed to this connector. Can be used to connect some small disply (SW not implemented yet)

J3 connector is for flashing and powering board on table. Also can be used for TunerStudion connection.

J1 and J2 are JTAG connectos.

## Uart pinout

J3 connector. Colors are default for cheap usb-to-uart converters based on CP2101 or FTDI232
Connect red wire if you want to power device from USB.
Default baud rate is 115200.

![image](https://user-images.githubusercontent.com/48498823/208742019-953c3ffc-588c-409b-8e2a-7ff916e8f506.png)

# Bluetooth

No extra steps needed to initialize JDY-33. Default device name is "RusEFI WBO x2" (or "RuseEFI WBO x2 BLE" for BT BLE)

Default baud rate is 115200.

# TunerStudio configuration

Baudrate is 115200
[.ini file](https://github.com/dron0gus/wideband/blob/master/firmware/ini/rusefi_wb_f1_dual.ini) for both UART and BT connection.

Both interfaces can be used simultaneously for data logging but not for settings change (from rev1).

Rev0 share same UART for both interfaces.

# Firmware Upload

Two ways to program using STM32CubeProgrammer

* recommended way: using UART connectivity. Power device up while shorting BOOT0 jumper to enter DFU mode (see "DFU mode/floating PB2" issue above)
* st-link if you have tc2030 spring-loaded cable.
* update over CAN or UART (J3) using OpenBLT (update over BT is in progress)

TL,DR: 9600 nothing connected on main connector

![image](https://github.com/rusefi/rusefi-hardware/assets/48498823/dac36691-878c-45d1-a2d9-7ee26409be4e)

## Updating over UART using DFU mode

1. Disconnect main connector. If you are going to apply +12V power through main connector - make sure that nothing else is connected to WBO (LSU sensors, any load, etc). But I recommend flash using +5V from USB.
   
2. connect USB to uart connector to J3 connector: gnd, rx and tx.
Don't forget to cross Rx-`Tx (adapter's Tx goes to WBO's Rx, WBO's Tx goes to adapter's Rx).
If you going to power WBO from USB port - also attach 5V line. Do not connect +5 from USB adapter if you are going to use +12V supply through main connector.

![20230923_125247](https://github.com/rusefi/rusefi-hardware/assets/28624689/04b9595a-4832-4e9d-92f6-b6955232a969)

3. Download and install STM32 Flash Loader Demonstrator. (Alternative tool is stm32flash - not covered in this instruction)

4. Figure out USB to serial serial port number: 

![device manager](https://github.com/rusefi/rusefi-hardware/assets/28624689/6bf0a0fc-8c34-4061-bd9a-e3c64491b45d)

5. Start Flash Loader Demonstrator GUI application, select correct COM port. Optional: reduce timeout to 1 second:

![stm32 flasher](https://github.com/rusefi/rusefi-hardware/assets/28624689/b5a6767c-59d1-41b7-a17b-9218185aa7b0)

6. Press and hold BOOT0 button on the bottom of PCB. Or short BOOT0 PCB jumper if button is not populated on your board.

7. Apply power (while holding BOOT0 button) to board and press Next in application. After app detects chip you should see something similar to:

![stm32 flasher 2](https://github.com/rusefi/rusefi-hardware/assets/28624689/bc78d3d1-68d1-4648-b5f3-19e4711ce60b)

8. Press next, select "Download to device", select wideband.bin file. Select "Jump to user application" and "Verify after download"

![stm32 flasher 3](https://github.com/rusefi/rusefi-hardware/assets/28624689/aa6137e8-ad5c-4747-9fae-2e42df8d1ff3)

9. Press Next and wait for flash/verification ends.

![stm32 flasher 4](https://github.com/rusefi/rusefi-hardware/assets/28624689/329a686d-6a6e-42fe-9674-fb4d2ad88d72)

## Updating using JTAG

DBD.

## Update using OpenBLT

OpenBLT https://www.feaser.com/openblt/doku.php

1. Restart device to OpenBLT mode using "Reset to OpenBLT" button under Controller -> ECU tools in TunerStudio.
   
2. Close TunerStudio to release serial port

3. Open BootCommander, select correct serial port, select wideband_update.srec file.

# Changelog

## rev 1
* Switch to 64 pin STM32/GD32
* Add 8MHz HSE oscillator (better clock stability for CAN over wide temperature range)
* Use DAC (instead of PWM) outputs for driving Ip
* Move BT to UART3/USART3, keep USART1 for bootloader/debug
* Drive BT's EN signal from MCU
* Fix RC filters on heater outputs for better voltage measurement
* Use separate ADC input for Vbat measurement
* Use ADC channels for AUX outputs monitoring/diagnostic
* Use PWM noise cancelation circuit on AUX analog outputs
* Removed useless/non-functional protection of Vm outputs
* Fix board outline to fit two types of enclosure ("plastic top" and "alloy brick")
* Add testpoints for TC2030 JTAG probe
* Add buttons for Reset and Boot0
* Add Cfg1 and jumpers
* Replace fuse holder with resettable 0.2A 33V fuse
* Two status LEDs for separate status indication for left and right channel
* Silkscreen labels around testpoints
* Silkscreen labels at uart connector on both sides of PCB
* Un_3x_sense formula is now 0.247 + 3.15 * (Un - Vm)
* Un_sense (not gained) is also routed to ADC input
* Protection from possible Ip current when MCU in reset/bootloader
* White "power on" LED replaced with green
* Main connector pinout added to top silkscreen
* AUX connector pinout added to top silkscreen

## rev 0

Seems to work with proper C10/C35 using https://github.com/rusefi/wideband/releases/tag/20230210-does-not-match-binaris
