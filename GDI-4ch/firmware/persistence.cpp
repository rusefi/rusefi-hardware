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

void ReadOrDefault() {
    size_t size = sizeof(GDIConfiguration);
    mfs_error_t err = mfsReadRecord(&mfs1, MFS_RECORD_ID, &size, (uint8_t*)&configuration);
    if (err == MFS_NO_ERROR && configuration.version == PERSISTENCE_VERSION) {
        return;
    } else {
        configuration.resetToDefaults();
    }
}

void saveConfiguration() {
  configuration.updateCounter++;
  mfs_error_t writeErr = mfsWriteRecord(&mfs1, MFS_RECORD_ID, sizeof(GDIConfiguration), (uint8_t*)&configuration);
}

uint16_t float2short100(float value) {
    return FIXED_POINT * value;
}

float short2float100(uint16_t value) {
    return value / 1.0 / FIXED_POINT;
}
