
#pragma once

// We need these helpers because the frame layout is different on STM32H7
#ifdef STM32H7XX
#define CAN_SID(f) ((f).std.SID)
#define CAN_EID(f) ((f).ext.EID)
#define CAN_ISX(f) ((f).common.XTD)
#else
#define CAN_SID(f) ((f).SID)
#define CAN_EID(f) ((f).EID)
#define CAN_ISX(f) ((f).IDE)
#endif

#define getVoltageFrom8Bit(b) (5.0f * (b) / 255)

void initCan();
void startNewCanTest();
bool isHappyCanTest();
bool checkDigitalInputCounterStatus();
int getDigitalOutputStepsCount();
int getDigitalDcOutputStepsCount();
int getLowSideOutputCount();
void setOutputCountRequest();
void sendCanPinState(uint8_t pinIdx, bool isSet);
void sendCanDcState(uint8_t index, bool isSet);

