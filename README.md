# rusefi-hardware

open source repo just for rusEFI Hardware

See also https://github.com/rusefi/rusefi/tree/master/hardware

# Breakouts

Many adapters are available assembled at https://shop.rusefi.com

## Connect into wires

Connector on one side, screw terminals or pads for your own harness on the other

* [24 pin](breakout-boards/Breakout-24pin) generic 24 pin to three connectors
* [135 pin](breakout-boards/Breakout-135-subaru-single) Subaru single connector (3D models only)
* [154 pin](breakout-boards/Breakout_154pin_284617-1-Connector) Volkswagen Audi VAG 284617-1

More "connect into wires" breakouts (35 to 134 pin: Mazda, Chrysler NGC, Mitsubishi, Motronic, Ford EEC-IV, VAG, BMW) live in the main repo at https://github.com/rusefi/rusefi/tree/master/hardware

## Superseal adapters

Vehicle-specific adapters keep the original harness intact: stock ECU connector on one side, superseal into a rusEFI ECU on the other

* [24 pin superseal](breakout-boards/Breakout-24pin-superseal) same as 24 pin but going into superseal
* [112 pin](breakout-boards/adapter-board-me17-112-B-schematic.pdf) Bosch ME17 ([rev A](breakout-boards/adapter-board-me17-112-A-schematic.pdf))
* [121 pin](breakout-boards/Ferrari-Maserati-121-adapter-rev-a.pdf) Ferrari/Maserati F136 engine
* [154 pin](breakout-boards/Breakout_154_kia_pb) Hyundai/Kia 1.6 GDI PB ([pinout](https://rusefi.com/docs/pinouts/hellen/hellen-hyundai-pb-mt/))
* [198 pin](breakout-boards/Breakout-198-Mustang) Ford Mustang Coyote ([pinout](https://rusefi.com/docs/pinouts/hellen/coyote/))
* [Mazda Miata NC](breakout-boards/Breakout-mazda-nc-0.1.pdf) ([pinout](https://rusefi.com/docs/pinouts/miata-nc/))
* [Mitsubishi Lancer Evo X](breakout-boards/mitsubishi-lancer-evo-x-rev-a.pdf)
* [Mitsubishi Mirage](breakout-boards/mitsubishi-mirage-adapter-0.3.pdf)
* [Subaru 2011](subaru-2011-adapter-docs) ([vehicle pinout](https://rusefi.com/docs/pinouts/subaru-2011/), [adapter pinout](https://rusefi.com/docs/pinouts/subaru%20adapter/))
* [154 pin](breakout-boards/Passat-2.0-MED9.1-adapter.pdf) Volkswagen Passat 2.0 MED9.1 GDI

## Other

* [ZF 8HP](ZF8HP%20Transmission) transmission installation board, solders in place of the stock TCU

Vehicle connector pinouts (no board, just `connectors/*.yaml` for [interactive pinouts](https://rusefi.com/docs/pinouts/)):

* [BMW N52](BMW-N52-146) 146 pin ([pinout](https://rusefi.com/docs/pinouts/BMW-N52-146/))
* [BMW N54](BMW-N54-146) 146 pin ([pinout](https://rusefi.com/docs/pinouts/BMW-N54-146/))
* [Honda OBD1](honda-obd1) 64 pin + 76 pin ([pinout](https://rusefi.com/docs/pinouts/uaefi/honda-obd1/))
* [Toyota JZA80](Toyota-JZA80) Supra MK4 E9/E10
* [Toyota JZA90](Toyota-JZA90) Supra MK5
* [Jatco](jatco) transmission
* [Kawasaki](kawasaki) motorcycle

# Modules

* [A4988](A4988_stepper_motor_driver) stepper motor driver
* [L9779WD](L9779WD-breakout) ASIC breakout
* [MC33810](MC33810-breakout) ASIC breakout
* [TLE9104](tle9104-breakout) ASIC breakout
* [LM1949](Low-Z_LM1949) low impedance injector driver
* [IR2302](IR2302-testboard) test board
* [VR-Hall](VR-Hall) VR/Hall conditioner
* [NCV1124](VR_ncv1124_test_module) VR test module
* [frequency divider](frequency-divider)
* [quad IGBT](quad-igbt) and [superseal IGBT](superseal-igbt) ignition drivers
* [stm32 Brain board 48pin](mini48-stm32)
* [uaBrain GM E38](uaBrain-gm-e38) hw-uaBrain module carrier for GM E38 (draft)
* [GDI-4ch](GDI-4ch), [GDI-6ch](GDI-6ch), [GDI-STM](GDI-STM) direct injection drivers
* [lambda-x2](lambda-x2), [wideband-F103](wideband-F103) wideband controllers
* [CDI-test](CDI-test), [stim](stim), [digital-inputs](digital-inputs) test and QC boards

# Classic designs

Frankenso, Frankenstein and other legacy designs are in [classic-designs](classic-designs)

# Important note

Depends on libraries from https://github.com/rusefi/kicad-libraries

If you git clone be sure to also

`git submodule update --init`
