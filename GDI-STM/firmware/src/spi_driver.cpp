#include "spi_driver.hpp"

#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_spi.h"

#include <cstring>

extern SPI_HandleTypeDef hspi1;

SpiDriver::SpiDriver() : initialized_(false), lastError_(0) {}

SpiDriver::~SpiDriver() = default;

bool SpiDriver::init() {
    // No SPI device on current PCB revision.
    initialized_ = false;
    return false;
}

bool SpiDriver::configureInjector(const SpiInjectorConfig& config) {
    if (!initialized_) {
        return false;
    }

    uint8_t txBuffer[16] = {0};
    uint8_t rxBuffer[16] = {0};

    txBuffer[0] = CMD_CONFIG_INJECTOR;
    txBuffer[1] = config.injectorId;
    txBuffer[2] = static_cast<uint8_t>((config.peakCurrent >> 8) & 0xFF);
    txBuffer[3] = static_cast<uint8_t>(config.peakCurrent & 0xFF);
    txBuffer[4] = static_cast<uint8_t>((config.holdCurrent >> 8) & 0xFF);
    txBuffer[5] = static_cast<uint8_t>(config.holdCurrent & 0xFF);
    txBuffer[6] = static_cast<uint8_t>((config.peakTime >> 8) & 0xFF);
    txBuffer[7] = static_cast<uint8_t>(config.peakTime & 0xFF);
    txBuffer[8] = config.diagnosticsMode;
    txBuffer[9] = config.faultThreshold;

    const uint16_t crc = calculateCrc(txBuffer, 10);
    txBuffer[10] = static_cast<uint8_t>((crc >> 8) & 0xFF);
    txBuffer[11] = static_cast<uint8_t>(crc & 0xFF);

    if (!selectInjector(config.injectorId)) {
        lastError_ = 0x01;
        return false;
    }

    const bool success = transferData(txBuffer, rxBuffer, 12);
    deselectInjector();

    if (!success) {
        lastError_ = 0x02;
        return false;
    }

    if (rxBuffer[0] != 0xAA) {
        lastError_ = 0x03;
        return false;
    }

    return true;
}

bool SpiDriver::readInjectorStatus(uint8_t injectorId, uint32_t& status) {
    if (!initialized_) {
        return false;
    }

    uint8_t txBuffer[8] = {0};
    uint8_t rxBuffer[8] = {0};

    txBuffer[0] = CMD_READ_STATUS;
    txBuffer[1] = injectorId;

    const uint16_t crc = calculateCrc(txBuffer, 2);
    txBuffer[2] = static_cast<uint8_t>((crc >> 8) & 0xFF);
    txBuffer[3] = static_cast<uint8_t>(crc & 0xFF);

    if (!selectInjector(injectorId)) {
        lastError_ = 0x01;
        return false;
    }

    const bool success = transferData(txBuffer, rxBuffer, 8);
    deselectInjector();

    if (!success) {
        lastError_ = 0x02;
        return false;
    }

    status = (static_cast<uint32_t>(rxBuffer[1]) << 24) | (static_cast<uint32_t>(rxBuffer[2]) << 16) |
             (static_cast<uint32_t>(rxBuffer[3]) << 8) | static_cast<uint32_t>(rxBuffer[4]);

    return true;
}

bool SpiDriver::performInjectorDiagnostics(uint8_t injectorId, uint8_t* outData, size_t outCapacity, size_t& outWritten) {
    outWritten = 0;
    if (!initialized_ || !outData || outCapacity == 0) {
        return false;
    }

    uint8_t txBuffer[4] = {0};
    uint8_t rxBuffer[32] = {0};

    txBuffer[0] = CMD_DIAGNOSTICS;
    txBuffer[1] = injectorId;

    const uint16_t crc = calculateCrc(txBuffer, 2);
    txBuffer[2] = static_cast<uint8_t>((crc >> 8) & 0xFF);
    txBuffer[3] = static_cast<uint8_t>(crc & 0xFF);

    if (!selectInjector(injectorId)) {
        lastError_ = 0x01;
        return false;
    }

    const bool success = transferData(txBuffer, rxBuffer, sizeof(rxBuffer));
    deselectInjector();

    if (!success) {
        lastError_ = 0x02;
        return false;
    }

    // rxBuffer[0] is assumed to be an ACK/status byte.
    const size_t payloadLen = sizeof(rxBuffer) - 1;
    const size_t toCopy = (outCapacity < payloadLen) ? outCapacity : payloadLen;
    memcpy(outData, rxBuffer + 1, toCopy);
    outWritten = toCopy;
    return true;
}

bool SpiDriver::setGlobalConfig(uint16_t voltageMv, uint8_t tempLimitC) {
    if (!initialized_) {
        return false;
    }

    uint8_t txBuffer[8] = {0};
    uint8_t rxBuffer[8] = {0};

    txBuffer[0] = CMD_GLOBAL_CONFIG;
    txBuffer[1] = static_cast<uint8_t>((voltageMv >> 8) & 0xFF);
    txBuffer[2] = static_cast<uint8_t>(voltageMv & 0xFF);
    txBuffer[3] = tempLimitC;

    const uint16_t crc = calculateCrc(txBuffer, 4);
    txBuffer[4] = static_cast<uint8_t>((crc >> 8) & 0xFF);
    txBuffer[5] = static_cast<uint8_t>(crc & 0xFF);

    const bool success = transferData(txBuffer, rxBuffer, 8);
    if (!success) {
        lastError_ = 0x02;
        return false;
    }

    return (rxBuffer[0] == 0xAA);
}

uint32_t SpiDriver::lastError() const {
    return lastError_;
}

void SpiDriver::handleSpiErrors() {
    if (HAL_SPI_GetError(&hspi1) != HAL_SPI_ERROR_NONE) {
        (void)HAL_SPI_DeInit(&hspi1);
    }
}

bool SpiDriver::transferData(const uint8_t* txData, uint8_t* rxData, uint16_t length) {
    const HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi1, const_cast<uint8_t*>(txData), rxData, length, 100);
    if (status != HAL_OK) {
        handleSpiErrors();
        return false;
    }
    return true;
}

bool SpiDriver::selectInjector(uint8_t) {
    return true;
}

void SpiDriver::deselectInjector() {}

uint16_t SpiDriver::calculateCrc(const uint8_t* data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= static_cast<uint16_t>(data[i]) << 8;
        for (uint8_t j = 0; j < 8; j++) {
            crc = (crc & 0x8000) ? static_cast<uint16_t>((crc << 1) ^ 0x1021) : static_cast<uint16_t>(crc << 1);
        }
    }
    return crc;
}
