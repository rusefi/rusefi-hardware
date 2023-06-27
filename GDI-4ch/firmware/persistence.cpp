#include "ch.h"
#include "hal.h"

#include "persistence.h"

#define MFS_RECORD_ID     1

const MFSConfig mfscfg1 = {
  .flashp           = (BaseFlash *)&EFLD1,
  .erased           = 0xFFFFFFFFU,
  .bank_size        = 1024U,
  .bank0_start      = 62U,
  .bank0_sectors    = 1U,
  .bank1_start      = 63U,
  .bank1_sectors    = 1U
};

static MFSDriver mfs1;
uint8_t mfs_buffer[512];

/**
 * @return true if mfsStart is well
 */
mfs_error_t InitFlash() {
  mfsObjectInit(&mfs1);
  mfs_error_t err = mfsStart(&mfs1, &mfscfg1);
  return err;
}

extern GDIConfiguration configuration;
extern mfs_error_t flashState;

static uint8_t *GetConfigurationPtr() {
    return (uint8_t *)&configuration;
}

static size_t GetConfigurationSize() {
    return sizeof(GDIConfiguration);
}

void ReadOrDefault() {
    size_t size = GetConfigurationSize();
    flashState = mfsReadRecord(&mfs1, MFS_RECORD_ID, &size, GetConfigurationPtr());
    if (flashState == MFS_NO_ERROR && configuration.version == PERSISTENCE_VERSION) {
        return;
    } else {
        configuration.resetToDefaults();
    }
}

void saveConfiguration() {
  configuration.updateCounter++;
  flashState = mfsWriteRecord(&mfs1, MFS_RECORD_ID, GetConfigurationSize(), GetConfigurationPtr());
}

uint16_t float2short128(float value) {
    return FIXED_POINT * value;
}

float short2float128(uint16_t value) {
    return value / 1.0 / FIXED_POINT;
}
