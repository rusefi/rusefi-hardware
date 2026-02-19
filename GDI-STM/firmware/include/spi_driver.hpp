// No SPI device on current PCB revision. Protocol code retained for future use.

#ifndef SPI_DRIVER_HPP
#define SPI_DRIVER_HPP

#include <cstddef>
#include <cstdint>

// SPI protocol commands for injector driver IC (placeholder values).
enum : uint8_t {
    CMD_CONFIG_INJECTOR = 0x10,
    CMD_READ_STATUS = 0x20,
    CMD_DIAGNOSTICS = 0x30,
    CMD_GLOBAL_CONFIG = 0x40,
};

struct SpiInjectorConfig {
    uint8_t injectorId;
    uint16_t peakCurrent;
    uint16_t holdCurrent;
    uint16_t peakTime;
    uint8_t diagnosticsMode;
    uint8_t faultThreshold;
};

class SpiDriver {
public:
    SpiDriver();
    ~SpiDriver();

    bool init();

    bool configureInjector(const SpiInjectorConfig& config);
    bool readInjectorStatus(uint8_t injectorId, uint32_t& status);

    // Writes up to outCapacity bytes into outData and returns how many bytes were written.
    bool performInjectorDiagnostics(uint8_t injectorId, uint8_t* outData, size_t outCapacity, size_t& outWritten);

    bool setGlobalConfig(uint16_t voltageMv, uint8_t tempLimitC);

    uint32_t lastError() const;

private:
    bool initialized_;
    uint32_t lastError_;

    void handleSpiErrors();
    bool transferData(const uint8_t* txData, uint8_t* rxData, uint16_t length);

    bool selectInjector(uint8_t injectorId);
    void deselectInjector();

    static uint16_t calculateCrc(const uint8_t* data, uint16_t length);
};

#endif // SPI_DRIVER_HPP
