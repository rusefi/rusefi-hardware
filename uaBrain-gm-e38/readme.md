# uaBrain-gm-e38

GM E38 plug-and-play carrier for the uaBrain module.
Status: **initial draft, not routed, not built**.

## J1: 80-pin grey connector (Molex 31387-4018)

| Pin | E38 function (hellen-gm-e38 rev c) | uaBrain pad |
|-----|-----|-----|
| 1 |  |  |
| 2 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 |
| 3 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 |
| 4 |  |  |
| 5 | OUT_ETB+ | E2 |
| 6 | OUT_ETB- | E1 |
| 7 |  |  |
| 8 | OUT_EVAP_PURGE | G6 |
| 9 | OUT_DOD_CYL3 | - (not wired) |
| 10 | OUT_DOD_CYL1 | - (not wired) |
| 11 | OUT_DOD_CYL2 | - (not wired) |
| 12 | OUT_BK1S1HTR | M5 |
| 13 | OUT_BK2S1HTR | M6 |
| 14 | OUT_DOD_CYL4 | - (not wired) |
| 15 |  |  |
| 16 | OUT_VVT_HS | - (not wired) |
| 17 | OUT_INJ8 | - (not wired) |
| 18 | OUT_INJ5 | J5 |
| 19 | OUT_INJ6 | J6 |
| 20 | OUT_INJ1 | B4 |
| 21 | IN_CLT | E7 |
| 22 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 |
| 23 | IN_ATF_TEMP | - (not wired) |
| 24 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 |
| 25 |  |  |
| 26 | IN_KNOCK1 | B7 |
| 27 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 |
| 28 |  |  |
| 29 | IN_KNOCK2 | J7 |
| 30 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 |
| 31 |  |  |
| 32 | P1_SPARE | - (not wired) |
| 33 | IN_OIL_LEVEL_SWITCH | - (not wired) |
| 34 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 |
| 35 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 |
| 36 |  |  |
| 37 | OUT_INJ2 | B5 |
| 38 | OUT_INJ3 | B6 |
| 39 | OUT_INJ4 | J4 |
| 40 | OUT_INJ7 | - (not wired) |
| 41 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 |
| 42 |  |  |
| 43 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 |
| 44 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 |
| 45 |  |  |
| 46 |  |  |
| 47 |  |  |
| 48 |  |  |
| 49 |  |  |
| 50 | IN_OIL_PRESSURE | D2 |
| 51 |  |  |
| 52 | GND | C3, C8, G8, K3, K8, M3, M4, P5 |
| 53 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 |
| 54 |  |  |
| 55 |  |  |
| 56 | GND | C3, C8, G8, K3, K8, M3, M4, P5 |
| 57 | IN_BK1S1SIG | - (not wired) |
| 58 | IN_MAP | F6 |
| 59 |  |  |
| 60 | GND | C3, C8, G8, K3, K8, M3, M4, P5 |
| 61 | OUT_ALT_LIGHT | - (not wired) |
| 62 |  |  |
| 63 | IN_TPS2 | E4 |
| 64 | IN_CAM | G3 |
| 65 | IN_TPS1 | E3 |
| 66 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 |
| 67 |  |  |
| 68 | IN_CRANK | C4 |
| 69 | GND | C3, C8, G8, K3, K8, M3, M4, P5 |
| 70 | OUT_IGN1 | B1 |
| 71 | OUT_IGN8 | - (not wired) |
| 72 | OUT_IGN7 | - (not wired) |
| 73 | OUT_IGN2 | B2 |
| 74 | OUT_IGN6 | J3 |
| 75 | OUT_IGN5 | J2 |
| 76 | OUT_IGN4 | J1 |
| 77 | OUT_IGN3 | B3 |
| 78 | GND | C3, C8, G8, K3, K8, M3, M4, P5 |
| 79 | GND | C3, C8, G8, K3, K8, M3, M4, P5 |
| 80 |  |  |

## J2: 73-pin black connector (Molex 31387-2014)

| Pin | E38 function (hellen-gm-e38 rev c) | uaBrain pad |
|-----|-----|-----|
| 1 | IN_P/N_SWITCH | L2 |
| 2 |  |  |
| 3 |  |  |
| 4 |  |  |
| 5 |  |  |
| 6 | IN_CLUTCH_SWITCH | N8 |
| 7 |  |  |
| 8 | IN_REVERSE_SWITCH | - (not wired) |
| 9 | IN_STOP_SWITCH | N7 |
| 10 |  |  |
| 11 |  |  |
| 12 | IN_AC_PRESSURE | - (not wired) |
| 13 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 |
| 14 |  |  |
| 15 |  |  |
| 16 | IN_PRIM_FUEL_LEVEL | - (not wired) |
| 17 | OUT_FAN1_RELAY | M8 |
| 18 |  |  |
| 19 | 12V_KEY | M2 |
| 20 | +12V | M1 |
| 21 |  |  |
| 22 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 |
| 23 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 |
| 24 | IN_FTPS | - (not wired) |
| 25 |  |  |
| 26 | IN_CLUTCH_POSITION | - (not wired) |
| 27 | CAN_L | N6 |
| 28 | CAN_H | N5 |
| 29 | IN_PPS1 | N1 |
| 30 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 |
| 31 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 |
| 32 | IN_PPS2 | N2 |
| 33 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 |
| 34 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 |
| 35 |  |  |
| 36 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 |
| 37 | IN_IAT | G1 |
| 38 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 |
| 39 |  |  |
| 40 | IN_DIN1 | P6 |
| 41 | IN_MAF | F3 |
| 42 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 |
| 43 |  |  |
| 44 |  |  |
| 45 |  |  |
| 46 |  |  |
| 47 | +12V_RAW | A6, H6 |
| 48 | OUT_TACH | L1 |
| 49 |  |  |
| 50 | OUT_FUEL_PUMP1_RELAY_HS | - (not wired) |
| 51 | OUT_FUEL_PUMP2_RELAY_HS | - (not wired) |
| 52 | OUT_STARTER_RELAY_HS | - (not wired) |
| 53 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 |
| 54 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 |
| 55 |  |  |
| 56 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 |
| 57 | OUT_VSS_HS | - (not wired) |
| 58 | OUT_FAN2_RELAY | C7 |
| 59 | OUT_MAIN_RELAY | M7 |
| 60 |  |  |
| 61 | OUT_EVAP_VENT | - (not wired) |
| 62 |  |  |
| 63 | OUT_AC_RELAY | D1 |
| 64 |  |  |
| 65 |  |  |
| 66 | OUT_SKIP_SHIFT_SOLENOID | - (not wired) |
| 67 | OUT_STARTER_RELAY | G7 |
| 68 | OUT_CHECK_ENGINE | K7 |
| 69 |  |  |
| 70 | IN_SEC_FUEL_LEVEL | - (not wired) |
| 71 | IN_VSS+ | K4 |
| 72 | GND | C3, C8, G8, K3, K8, M3, M4, P5 |
| 73 | GND | C3, C8, G8, K3, K8, M3, M4, P5 |

## hw-uaBrain pads left unconnected

A1, A2, A3, A4, A5, A7, A8, C1, C2, D5, D6, D7, D8, F1, F2, G4, G5, H1, H2, H3, H4, H5, H7, H8, K1, K2, L5, L6, L7, L8, P1, P2, P3, P4
