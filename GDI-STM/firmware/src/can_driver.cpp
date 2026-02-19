#include "can_driver.hpp"

#include <cstring>

extern "C" void Error_Handler(void);

static CanDriver* g_canDriverInstance = nullptr;

static uint32_t dlcFromLen(uint8_t len) {
    switch (len) {
        case 0: return FDCAN_DLC_BYTES_0;
        case 1: return FDCAN_DLC_BYTES_1;
        case 2: return FDCAN_DLC_BYTES_2;
        case 3: return FDCAN_DLC_BYTES_3;
        case 4: return FDCAN_DLC_BYTES_4;
        case 5: return FDCAN_DLC_BYTES_5;
        case 6: return FDCAN_DLC_BYTES_6;
        case 7: return FDCAN_DLC_BYTES_7;
        default: return FDCAN_DLC_BYTES_8;
    }
}

static uint8_t lenFromDlc(uint32_t dlc) {
    switch (dlc) {
        case FDCAN_DLC_BYTES_0: return 0;
        case FDCAN_DLC_BYTES_1: return 1;
        case FDCAN_DLC_BYTES_2: return 2;
        case FDCAN_DLC_BYTES_3: return 3;
        case FDCAN_DLC_BYTES_4: return 4;
        case FDCAN_DLC_BYTES_5: return 5;
        case FDCAN_DLC_BYTES_6: return 6;
        case FDCAN_DLC_BYTES_7: return 7;
        default: return 8;
    }
}

CanDriver::CanDriver() : rxQueue_(nullptr), busErrorCount_(0) {
    hfdcan1_.Instance = FDCAN1;
    hfdcan1_.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    hfdcan1_.Init.Mode = FDCAN_MODE_NORMAL;
    hfdcan1_.Init.AutoRetransmission = ENABLE;
    hfdcan1_.Init.TransmitPause = DISABLE;
    hfdcan1_.Init.ProtocolException = DISABLE;

    // Default nominal bit timing: 500 kbps with FDCAN kernel clock = PCLK1 = 170 MHz.
    // Bitrate = 170 MHz / (Prescaler * (1 + TSeg1 + TSeg2)) = 170e6 / (17 * 20) = 500e3.
    hfdcan1_.Init.NominalPrescaler = 17;
    hfdcan1_.Init.NominalSyncJumpWidth = 2;
    hfdcan1_.Init.NominalTimeSeg1 = 13;
    hfdcan1_.Init.NominalTimeSeg2 = 6;

    // Data phase unused for Classic CAN.
    hfdcan1_.Init.DataPrescaler = 1;
    hfdcan1_.Init.DataSyncJumpWidth = 1;
    hfdcan1_.Init.DataTimeSeg1 = 1;
    hfdcan1_.Init.DataTimeSeg2 = 1;

    hfdcan1_.Init.StdFiltersNbr = 0;
    hfdcan1_.Init.ExtFiltersNbr = 1;
    hfdcan1_.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
}

CanDriver::~CanDriver() {
    (void)HAL_FDCAN_DeInit(&hfdcan1_);
    if (rxQueue_) {
        vQueueDelete(rxQueue_);
        rxQueue_ = nullptr;
    }
    if (g_canDriverInstance == this) {
        g_canDriverInstance = nullptr;
    }
}

bool CanDriver::init() {
    if (!rxQueue_) {
        rxQueue_ = xQueueCreate(kRxQueueLength, sizeof(CanMessage));
        if (!rxQueue_) {
            return false;
        }
    }

    if (HAL_FDCAN_Init(&hfdcan1_) != HAL_OK) {
        return false;
    }

    // Accept rusEFI GDI4 config packets 0xBB30-0xBB34 (mask 0x1FFFFFF0)
    if (!configureExtendedFilter(0xBB30U, 0x1FFFFFF0U)) {
        return false;
    }

    g_canDriverInstance = this;
    startReception();

    // Enable bus error notifications.
    (void)HAL_FDCAN_ActivateNotification(&hfdcan1_,
        FDCAN_IT_BUS_OFF | FDCAN_IT_ERROR_PASSIVE | FDCAN_IT_ERROR_WARNING, 0);

    if (HAL_FDCAN_Start(&hfdcan1_) != HAL_OK) {
        return false;
    }
    return true;
}

bool CanDriver::sendMessage(const CanMessage& msg) {
    if (msg.length > 8) {
        return false;
    }

    FDCAN_TxHeaderTypeDef txHeader{};
    txHeader.Identifier = msg.isExtended ? (msg.id & 0x1FFFFFFFU) : (msg.id & 0x7FFU);
    txHeader.IdType = msg.isExtended ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID;
    txHeader.TxFrameType = FDCAN_DATA_FRAME;
    txHeader.DataLength = dlcFromLen(msg.length);
    txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    txHeader.BitRateSwitch = FDCAN_BRS_OFF;
    txHeader.FDFormat = FDCAN_CLASSIC_CAN;
    txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    txHeader.MessageMarker = 0;

    return (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1_, &txHeader, const_cast<uint8_t*>(msg.data)) == HAL_OK);
}

bool CanDriver::receiveMessage(CanMessage& msg, uint32_t timeoutMs) {
    if (!rxQueue_) {
        return false;
    }

    const TickType_t timeoutTicks = (timeoutMs == 0) ? 0 : pdMS_TO_TICKS(timeoutMs);
    return (xQueueReceive(rxQueue_, &msg, timeoutTicks) == pdTRUE);
}

bool CanDriver::setFilter(uint32_t filterId, uint32_t mask) {
    return configureFilter(filterId, mask);
}

bool CanDriver::setExtendedFilter(uint32_t filterId, uint32_t mask) {
    return configureExtendedFilter(filterId, mask);
}

bool CanDriver::configureExtendedFilter(uint32_t filterId, uint32_t mask) {
    FDCAN_FilterTypeDef sFilterConfig{};
    sFilterConfig.IdType = FDCAN_EXTENDED_ID;
    sFilterConfig.FilterIndex = 0;
    sFilterConfig.FilterType = FDCAN_FILTER_MASK;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sFilterConfig.FilterID1 = filterId & 0x1FFFFFFFU;
    sFilterConfig.FilterID2 = mask & 0x1FFFFFFFU;

    return (HAL_FDCAN_ConfigFilter(&hfdcan1_, &sFilterConfig) == HAL_OK);
}

FDCAN_HandleTypeDef* CanDriver::handle() {
    return &hfdcan1_;
}

bool CanDriver::configureFilter(uint32_t filterId, uint32_t mask) {
    FDCAN_FilterTypeDef sFilterConfig{};
    sFilterConfig.IdType = FDCAN_STANDARD_ID;
    sFilterConfig.FilterIndex = 0;
    sFilterConfig.FilterType = FDCAN_FILTER_MASK;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sFilterConfig.FilterID1 = filterId & 0x7FF;
    sFilterConfig.FilterID2 = mask & 0x7FF;

    return (HAL_FDCAN_ConfigFilter(&hfdcan1_, &sFilterConfig) == HAL_OK);
}

void CanDriver::startReception() {
    if (HAL_FDCAN_ActivateNotification(&hfdcan1_, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
        Error_Handler();
    }
}

void CanDriver::processReceivedMessages() {
    if (!rxQueue_) {
        return;
    }

    FDCAN_RxHeaderTypeDef rxHeader{};
    uint8_t rxData[8] = {0};

    if (HAL_FDCAN_GetRxMessage(&hfdcan1_, FDCAN_RX_FIFO0, &rxHeader, rxData) != HAL_OK) {
        return;
    }

    CanMessage msg{};
    msg.id = rxHeader.Identifier;
    msg.length = lenFromDlc(rxHeader.DataLength);
    msg.isExtended = (rxHeader.IdType == FDCAN_EXTENDED_ID);
    memcpy(msg.data, rxData, msg.length);

    BaseType_t woken = pdFALSE;
    (void)xQueueSendFromISR(rxQueue_, &msg, &woken);
    portYIELD_FROM_ISR(woken);
}

void CanDriver::onBusError() {
    busErrorCount_++;
}

uint32_t CanDriver::busErrorCount() const {
    return busErrorCount_;
}

extern "C" void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef* hfdcan, uint32_t ErrorStatusITs) {
    (void)ErrorStatusITs;
    if (g_canDriverInstance && hfdcan == g_canDriverInstance->handle()) {
        g_canDriverInstance->onBusError();
    }
}

extern "C" void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo0ITs) {
    if (!g_canDriverInstance) {
        return;
    }
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == 0) {
        return;
    }
    if (hfdcan == g_canDriverInstance->handle()) {
        g_canDriverInstance->processReceivedMessages();
    }
}

extern "C" void FDCAN1_IT0_IRQHandler(void) {
    if (g_canDriverInstance) {
        HAL_FDCAN_IRQHandler(g_canDriverInstance->handle());
    }
}

extern "C" void FDCAN1_IT1_IRQHandler(void) {
    if (g_canDriverInstance) {
        HAL_FDCAN_IRQHandler(g_canDriverInstance->handle());
    }
}
