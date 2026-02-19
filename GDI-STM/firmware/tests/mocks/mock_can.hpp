#ifndef MOCK_CAN_HPP
#define MOCK_CAN_HPP

#include <gmock/gmock.h>
#include <stdint.h>

// CAN message structure
struct CanMessage {
    uint32_t id;
    uint8_t length;
    uint8_t data[8];
};

// Mock CAN interface
class MockCan {
public:
    MOCK_METHOD(bool, init, ());
    MOCK_METHOD(bool, sendMessage, (const CanMessage& msg));
    MOCK_METHOD(bool, receiveMessage, (CanMessage& msg));
    MOCK_METHOD(bool, setFilter, (uint32_t filterId, uint32_t mask));
    MOCK_METHOD(void, processInterrupt, ());
};

#endif // MOCK_CAN_HPP
