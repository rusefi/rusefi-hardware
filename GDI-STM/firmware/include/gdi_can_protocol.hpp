#ifndef GDI_CAN_PROTOCOL_HPP
#define GDI_CAN_PROTOCOL_HPP

#include "can_driver.hpp"

// rusEFI GDI4 CAN protocol handler.
// Receives 0xBB30-0xBB34 from ECU, applies to boost/injector.
// Transmits 0xBB20-0xBB25 to ECU (status, config echo, version).
class GdiCanProtocol {
public:
    explicit GdiCanProtocol(CanDriver& can);

    // Process one received message. Call from main loop.
    void processReceivedMessage(const CanMessage& msg);

    // Periodic transmit. Call every ~100 ms from main loop.
    void periodicTransmit();

private:
    CanDriver& can_;
    uint32_t txCounter_{0};

    // Stored config from ECU (for echo and application)
    uint16_t boostVoltage_{0};
    uint16_t boostCurrent_{0};  // ×128
    uint16_t tBoostMin_{0};
    uint16_t tBoostMax_{0};
    uint16_t peakCurrent_{0};   // ×128
    uint16_t tpeakDuration_{0};
    uint16_t tpeakOff_{0};
    uint16_t tbypass_{0};
    uint16_t holdCurrent_{0};   // ×128
    uint16_t tholdOff_{0};
    uint16_t tholdDuration_{0};
    uint16_t pumpPeakCurrent_{0};  // ×128
    uint16_t pumpHoldCurrent_{0};  // ×128

    void setTwoBytesLsb(uint8_t* data, int offset, uint32_t value);
    void sendStatus();
    void sendConfig1();
    void sendConfig2();
    void sendConfig3();
    void sendConfig4();
    void sendVersion();
};

#endif // GDI_CAN_PROTOCOL_HPP
