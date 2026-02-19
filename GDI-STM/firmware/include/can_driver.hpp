#ifndef CAN_DRIVER_HPP
#define CAN_DRIVER_HPP

#include <cstdint>

#include "FreeRTOS.h"
#include "queue.h"

#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_fdcan.h"

struct CanMessage {
    uint32_t id;
    uint8_t length;
    uint8_t data[8];
    bool isExtended{false};
};

class CanDriver {
public:
    CanDriver();
    ~CanDriver();

    bool init();

    bool sendMessage(const CanMessage& msg);
    bool receiveMessage(CanMessage& msg, uint32_t timeoutMs = 0);

    bool setFilter(uint32_t filterId, uint32_t mask);
    bool setExtendedFilter(uint32_t filterId, uint32_t mask);

    FDCAN_HandleTypeDef* handle();

    void processReceivedMessages();
    void onBusError();

    uint32_t busErrorCount() const;

private:
    static constexpr uint32_t kRxQueueLength = 32;

    FDCAN_HandleTypeDef hfdcan1_;
    QueueHandle_t rxQueue_;
    uint32_t busErrorCount_;

    bool configureFilter(uint32_t filterId, uint32_t mask);
    bool configureExtendedFilter(uint32_t filterId, uint32_t mask);
    void startReception();
};

#endif // CAN_DRIVER_HPP
