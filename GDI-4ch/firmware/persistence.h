/**
 * @file persistence.h
 */



#pragma once

void InitFlash();
int IncAndGet();

#define PERSISTENCE_VERSION 1

struct GDIConfiguration {
    void resetToDefaults();
    int version;

    float BoostVoltage;
    float BoostCurrent;
   	float PeakCurrent;
    float HoldCurrent;
    
	uint16_t TpeakOff;
	uint16_t TpeakTot;
	uint16_t Tbypass;

	uint16_t TholdOff;
	uint16_t THoldTot;
	uint16_t TBoostMin;
	uint16_t TBoostMax;
    
    float PumpPeakCurrent;
    float PumpHoldCurrent;
    uint16_t PumpTholdOff;
    uint16_t PumpTholdTot;
};

#define FIXED_POINT 100

uint16_t float2short100(float value);
float short2float100(uint16_t value);
