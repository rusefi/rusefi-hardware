# Pin Map (from `GDI-STM.kicad_pcb`)

This document summarizes the labeled nets connected to the STM32 (`U7`) on the **produced PCB** and how the firmware currently treats them.

To regenerate/extract directly from the PCB:
- `python scripts/kicad_pcb_pinmap.py --pcb GDI-STM.kicad_pcb --mcu-ref U7`
- `python scripts/verify_pcb_vs_firmware.py`

## Injector Control (`INJ1..INJ8`)

These nets are mapped in firmware as **GPIO outputs** and default to **LOW** at boot.

These also route to the BANK sheet pins (`LS1/LS2`) as shown in the top-level schematic.

| Net | MCU Pin | Package Pin (LQFP-80) | BANK Sheet Pin |
|---:|:--------|:-----------------------|:---------------|
| INJ1 | PE8 | 31 | `BANK1:LS1` |
| INJ2 | PE9 | 32 | `BANK1:LS2` |
| INJ3 | PE10 | 33 | `BANK2:LS1` |
| INJ4 | PE11 | 34 | `BANK2:LS2` |
| INJ5 | PE12 | 35 | `BANK3:LS1` |
| INJ6 | PE13 | 36 | `BANK3:LS2` |
| INJ7 | PE14 | 37 | `BANK4:LS1` |
| INJ8 | PE15 | 38 | `BANK4:LS2` |

## START Signals (`START1..START8`)

These nets are configured in firmware as **GPIO inputs** (no pull) and are used by `InjectorBridge` to bridge incoming START pulses to `INJ1..INJ8` outputs.

| Net | MCU Pin | Package Pin (LQFP-80) |
|---:|:--------|:-----------------------|
| START1 | PA9 | 57 |
| START2 | PA10 | 58 |
| START3 | PB3 | 72 |
| START4 | PB4 | 73 |
| START5 | PA15 | 65 |
| START6 | PB5 | 74 |
| START7 | PB6 | 75 |
| START8 | PB9 | 78 |

## CAN (FDCAN1)

| Net | MCU Pin | Package Pin (LQFP-80) | Firmware Mode |
|---:|:--------|:-----------------------|:--------------|
| CAN_TX | PA12 | 60 | AF9 `FDCAN1_TX` |
| CAN_RX | PA11 | 59 | AF9 `FDCAN1_RX` |

## Power Switch Enables

From `bank.kicad_sch`, these nets feed **IRS21867S** gate-driver inputs (`HIN`) and should be treated as **digital control outputs**. For safety, firmware holds them **LOW** at boot (drivers disabled).

| Net | MCU Pin | Package Pin (LQFP-80) |
|---:|:--------|:-----------------------|
| HS_VBAT1 | PC6 | 52 |
| HS_VBAT2 | PC7 | 53 |
| HS_VBAT3 | PC8 | 54 |
| HS_VBAT4 | PC9 | 55 |
| HS_VBOOST1 | PC10 | 66 |
| HS_VBOOST2 | PC11 | 67 |
| HS_VBOOST3 | PC12 | 68 |
| HS_VBOOST4 | PC13 | 2 |

## Boost PWM (`BOOST_PWM`)

| Net | MCU Pin | Package Pin (LQFP-80) |
|---:|:--------|:-----------------------|
| BOOST_PWM | PD0 | 69 |

## Sense / Monitor Pins (Analog Mode)

These pins are configured as **analog inputs** (GPIO analog mode). A subset is sampled by the firmware (see below).

| Net | MCU Pin | Package Pin (LQFP-80) |
|---:|:--------|:-----------------------|
| Vboost | PE7 | 30 |
| Voboost | PA8 | 56 |
| Vo1 | PA2 | 14 |
| Vo2 | PA6 | 20 |
| Vo3 | PB1 | 25 |
| Vo4 | PB12 | 43 |
| VinM1 | PC5 | 23 |
| VinM2 | PA5 | 19 |
| VinM3 | PB2 | 26 |
| VinM4 | PB10 | 39 |
| VinMboost | PB15 | 46 |
| VinP1 | PA7 | 21 |
| VinP2 | PB0 | 24 |
| VinP3 | PB13 | 44 |
| VinP4 | PB11 | 42 |
| VinPboost | PC3 | 11 |
| Vinj1 | PC0 | 8 |
| Vinj2 | PC1 | 9 |
| Vinj3 | PC2 | 10 |
| Vinj4 | PA0 | 12 |
| Vinj5 | PA1 | 13 |
| Vinj6 | PA3 | 17 |
| Vinj7 | PA4 | 18 |
| Vinj8 | PC4 | 22 |

## ADC Sampling (Implemented)

Polling-based ADC sampling is implemented in `src/adc_sampler.cpp:1` (no DMA yet).

Signals currently sampled:
- `VinM1` (PC5) via ADC2 channel 11
- `VinPboost` (PC3) via ADC2 channel 9
- `Vinj8` (PC4) via ADC2 channel 5
- `Vinj3` (PC2) via ADC2 channel 8
- `Vinj2` (PC1) via ADC2 channel 7
- `Voboost` (PA8) via ADC5 channel 1

The sampler updates once per `DefaultTask` loop (every 10 ms) in `src/main.cpp:49`.
