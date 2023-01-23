[Interactive Pinout](https://rusefi.com/docs/pinouts/lambda-x2/)


See https://github.com/mck1117/wideband

See https://rusefi.com/forum/viewtopic.php?f=4&t=2314



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
