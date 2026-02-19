#include "gdi_can_protocol.hpp"
#include "boost_control.hpp"
#include "injector_control.hpp"

#include "FreeRTOS.h"

extern InjectorControl injectorControl;
#include "task.h"

#include <cstring>

namespace {

constexpr uint32_t GDI4_BASE_ADDRESS = 0xBB20U;
constexpr uint32_t GDI4_CHANGE_ADDRESS = 0xBB30U;
constexpr uint8_t GDI4_MAGIC = 0x67U;
constexpr uint8_t GDI4_CAN_SET_TAG = 0x78U;
constexpr uint32_t GDI4_FIXED_POINT = 128U;

// Build date for version packet (compile-time)
// __DATE__ format: "Feb 13 2025" -> year=2025, month=2, day=13
constexpr int kBuildYear = 2025;
constexpr int kBuildMonth = 2;
constexpr int kBuildDay = 13;

// ECU packet: data[0]=0x78, then 16-bit values LSB-first at bytes 1-2, 3-4, 5-6
uint16_t getTwoBytesLsb(const uint8_t* data, int byteOffset) {
    return static_cast<uint16_t>(data[byteOffset]) |
           (static_cast<uint16_t>(data[byteOffset + 1]) << 8);
}

} // namespace

GdiCanProtocol::GdiCanProtocol(CanDriver& can) : can_(can) {}

void GdiCanProtocol::setTwoBytesLsb(uint8_t* data, int offset, uint32_t value) {
    data[offset] = static_cast<uint8_t>(value & 0xFFU);
    data[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFFU);
}

void GdiCanProtocol::processReceivedMessage(const CanMessage& msg) {
    if (!msg.isExtended || msg.length < 7) {
        return;
    }

    // Only process ECU→GDI packets (0xBB30-0xBB34)
    if (msg.id < GDI4_CHANGE_ADDRESS || msg.id > GDI4_CHANGE_ADDRESS + 4U) {
        return;
    }

    if (msg.data[0] != GDI4_CAN_SET_TAG) {
        return;
    }

    const uint32_t packetIndex = msg.id - GDI4_CHANGE_ADDRESS;

    switch (packetIndex) {
    case 0: {
        boostVoltage_ = getTwoBytesLsb(msg.data, 1);
        boostCurrent_ = getTwoBytesLsb(msg.data, 3);
        tBoostMin_ = getTwoBytesLsb(msg.data, 5);
        // Apply boost: voltage 0-100 maps to duty 0-100%
        const uint32_t v = static_cast<uint32_t>(boostVoltage_);
        const uint8_t duty = static_cast<uint8_t>(v > 100U ? 100U : v);
        boost::setDutyPercent(duty);
        boost::enable();
        break;
    }
    case 1: {
        tBoostMax_ = getTwoBytesLsb(msg.data, 1);
        peakCurrent_ = getTwoBytesLsb(msg.data, 3);
        tpeakDuration_ = getTwoBytesLsb(msg.data, 5);
        break;
    }
    case 2: {
        tpeakOff_ = getTwoBytesLsb(msg.data, 1);
        tbypass_ = getTwoBytesLsb(msg.data, 3);
        holdCurrent_ = getTwoBytesLsb(msg.data, 5);
        // Map to injector timing: onTimeUs = tpeakDuration, offTimeUs = tbypass or tholdOff
        injectorControl.setGlobalTiming(tpeakDuration_, tbypass_ > 0 ? tbypass_ : 1000);
        break;
    }
    case 3: {
        tholdOff_ = getTwoBytesLsb(msg.data, 1);
        tholdDuration_ = getTwoBytesLsb(msg.data, 3);
        pumpPeakCurrent_ = getTwoBytesLsb(msg.data, 5);
        break;
    }
    case 4: {
        pumpHoldCurrent_ = getTwoBytesLsb(msg.data, 1);
        break;
    }
    default:
        break;
    }
}

void GdiCanProtocol::sendStatus() {
    CanMessage msg{};
    msg.id = GDI4_BASE_ADDRESS;
    msg.isExtended = true;
    msg.length = 8;
    std::memset(msg.data, 0, 8);
    msg.data[7] = GDI4_MAGIC;  // Required for rusEFI limp mode
    (void)can_.sendMessage(msg);
}

void GdiCanProtocol::sendConfig1() {
    CanMessage msg{};
    msg.id = GDI4_BASE_ADDRESS + 1;
    msg.isExtended = true;
    msg.length = 8;
    setTwoBytesLsb(msg.data, 0, boostVoltage_);
    setTwoBytesLsb(msg.data, 2, boostCurrent_);
    setTwoBytesLsb(msg.data, 4, tBoostMin_);
    setTwoBytesLsb(msg.data, 6, tBoostMax_);
    (void)can_.sendMessage(msg);
}

void GdiCanProtocol::sendConfig2() {
    CanMessage msg{};
    msg.id = GDI4_BASE_ADDRESS + 2;
    msg.isExtended = true;
    msg.length = 8;
    setTwoBytesLsb(msg.data, 0, peakCurrent_);
    setTwoBytesLsb(msg.data, 2, tpeakDuration_);
    setTwoBytesLsb(msg.data, 4, tpeakOff_);
    setTwoBytesLsb(msg.data, 6, tbypass_);
    (void)can_.sendMessage(msg);
}

void GdiCanProtocol::sendConfig3() {
    CanMessage msg{};
    msg.id = GDI4_BASE_ADDRESS + 3;
    msg.isExtended = true;
    msg.length = 8;
    setTwoBytesLsb(msg.data, 0, holdCurrent_);
    setTwoBytesLsb(msg.data, 2, tholdOff_);
    setTwoBytesLsb(msg.data, 4, tholdDuration_);
    setTwoBytesLsb(msg.data, 6, pumpPeakCurrent_);
    (void)can_.sendMessage(msg);
}

void GdiCanProtocol::sendConfig4() {
    CanMessage msg{};
    msg.id = GDI4_BASE_ADDRESS + 4;
    msg.isExtended = true;
    msg.length = 8;
    setTwoBytesLsb(msg.data, 0, pumpHoldCurrent_);
    std::memset(msg.data + 2, 0, 6);
    (void)can_.sendMessage(msg);
}

void GdiCanProtocol::sendVersion() {
    CanMessage msg{};
    msg.id = GDI4_BASE_ADDRESS + 5;
    msg.isExtended = true;
    msg.length = 8;
    std::memset(msg.data, 0, 8);
    msg.data[1] = static_cast<uint8_t>(kBuildYear / 100);
    msg.data[2] = static_cast<uint8_t>(kBuildYear % 100);
    msg.data[3] = static_cast<uint8_t>(kBuildMonth);
    msg.data[4] = static_cast<uint8_t>(kBuildDay);
    (void)can_.sendMessage(msg);
}

void GdiCanProtocol::periodicTransmit() {
    txCounter_++;

    // Send status (0xBB20) every cycle for limp mode
    sendStatus();

    // Rotate through config packets 1-4 and version (5) to match Lua setTickRate(10)
    constexpr int packetKinds = 6;
    switch (txCounter_ % packetKinds) {
    case 0:
        sendConfig1();
        break;
    case 1:
        sendConfig2();
        break;
    case 2:
        sendConfig3();
        break;
    case 3:
        sendConfig4();
        break;
    case 4:
        sendVersion();
        break;
    case 5:
        // Extra slot: send status again for robustness
        sendStatus();
        break;
    default:
        break;
    }
}
