/**
 * @file persistence.h
 */



#pragma once

void InitFlash();
int IncAndGet();

#define PERSIS

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