/**
 * @file persistence.h
 */



#pragma once

/**
 * @return true if OK, false if broken
 */
bool InitFlash();
void saveConfiguration();
void ReadOrDefault();

#define PERSISTENCE_VERSION 2

struct GDIConfiguration {
    void resetToDefaults();
    int version;
    int updateCounter;

    float BoostVoltage;
    float BoostCurrent;
   	float PeakCurrent;
    float HoldCurrent;
    
	uint16_t TpeakOff;
	uint16_t TpeakDuration;
	uint16_t Tbypass;

	uint16_t TholdOff;
	uint16_t THoldDuration;
	uint16_t TBoostMin;
	uint16_t TBoostMax;
    
    float PumpPeakCurrent;
    float PumpHoldCurrent;
    uint16_t PumpTholdOff;
    uint16_t PumpTholdTot;

    short inputCanID;
};

#define FIXED_POINT 100

uint16_t float2short100(float value);
float short2float100(uint16_t value);
