[Interactive Pinout](https://rusefi.com/docs/pinouts/lambda-x2/)

See https://github.com/mck1117/wideband

See [rusEFI forum: F103 dual channel wideband controller + EGT + 2 x AUX In + 2 Aux out](https://rusefi.com/forum/viewtopic.php?f=4&t=2314)



* STM32F103 dual channel wideband AFR controller supporting Bosch LSU4.9 (LSU4.2 and LSU_ADV work in progress)
* CAN and analog output
* x2 EGT input using MAX31855KASA or MAX31856
* x2 auxilary analog input: 0..5V with pull-up or pull-down (configurable by soldering resistor). One can be used to source 5V to external sensor.
* x2 2 auxilary output 0..5V. Also can be used for slow PWM signal output.
* x2 open drain outputs: BTS3028 (5A)
* Bluetooth TunerStudio connectivity

![x](https://rusefi.com/forum/download/file.php?id=9478)


See also https://github.com/rusefi/rusefi/wiki/WBO

# Bluetooth
No extra steps needed to initialize JDY-33.
Default baud rate is 115200.


# Firmware Upload

![image](https://user-images.githubusercontent.com/48498823/208742019-953c3ffc-588c-409b-8e2a-7ff916e8f506.png)


## Changelog

# rev 1
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
* While "power on" LED replaced with green
* Main connector pinout added to top silkscreen
* AUX connector pinout added to top silkscreen

# rev 0

Seems to work with proper C10/C35 using https://github.com/rusefi/wideband/releases/tag/20230210-does-not-match-binaris
