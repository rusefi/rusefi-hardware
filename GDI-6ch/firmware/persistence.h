/**
 * @file persistence.h
 */

#include "hal_mfs.h"

#pragma once

/**
 * @return true if OK, false if broken
 */
mfs_error_t InitFlash();
void saveConfiguration();
void ReadOrDefault();

#define PERSISTENCE_VERSION 7

struct GDIConfiguration {
    bool IsValid() const {
        return version == PERSISTENCE_VERSION;
    }
    void resetToDefaults();
    int version;
    int updateCounter;

    // CAN protocol: packet 0, offset 1
    uint16_t BoostVoltage;
    // CAN protocol: packet 0, offset 3
    // Amps
    float BoostCurrent;
    // CAN protocol: packet 0, offset 5
	uint16_t TBoostMin;
    // CAN protocol: packet 1, offset 1
	uint16_t TBoostMax;

    // CAN protocol: packet 1, offset 3
   	float PeakCurrent;
   	// CAN protocol: packet 1, offset 5
	uint16_t TpeakDuration;
    // CAN protocol: packet 2, offset 1
	uint16_t TpeakOff;
    // CAN protocol: packet 2, offset 3
	uint16_t Tbypass;

    // CAN protocol: packet 2, offset 5
    float HoldCurrent;
    // CAN protocol: packet 3, offset 1
	uint16_t TholdOff;
    // CAN protocol: packet 3, offset 3
	uint16_t THoldDuration;

    float PumpPeakCurrent;
    float PumpHoldCurrent;
    uint16_t PumpTholdOff;
    uint16_t PumpTholdTot;

    int inputCanID;
    int outputCanID;
};

#define FIXED_POINT 128

uint16_t float2short128(float value);
float short2float128(uint16_t value);
