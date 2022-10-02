#include "ch.h"
#include "hal.h"

#include "persistence.h"
#include "hal_mfs.h"

const MFSConfig mfscfg1 = {
  .flashp           = (BaseFlash *)&EFLD1,
  .erased           = 0xFFFFFFFFU,
  .bank_size        = 1024U,
  .bank0_start      = 126U,
  .bank0_sectors    = 1U,
  .bank1_start      = 127U,
  .bank1_sectors    = 1U
};

static MFSDriver mfs1;
uint8_t mfs_buffer[512];

void InitFlash() {
  mfsObjectInit(&mfs1);
  mfs_error_t err = mfsStart(&mfs1, &mfscfg1);
//  chDbgAssert(err == MFS_NO_ERROR, "initialization error with erased flash");

}

mfs_error_t readErr;
mfs_error_t writeErr;

/*
extern GDIConfiguration configuration;

void ReadOrDefault() {
    size_t size = sizeof(GDIConfiguration);
    mfs_error_t err = mfsReadRecord(&mfs1, 1, &size, (uint8_t*)&configuration);
    if (err == MFS_NO_ERROR && configuration.version == PERSISTENCE_VERSION) {
        return;
    } else {
        configuration.resetToDefaults();
    }
}
*/

int IncAndGet() {
  size_t size = sizeof mfs_buffer;
  readErr = mfsReadRecord(&mfs1, 1, &size, mfs_buffer);
  int result;
  if (readErr == MFS_NO_ERROR) {
    result = 5;
  } else {
    result = mfs_buffer[0];
  }
  result++;
  mfs_buffer[0] = result;
  size = sizeof mfs_buffer;
  writeErr = mfsWriteRecord(&mfs1, 1, sizeof mfs_buffer, mfs_buffer);
  return result;
}

uint16_t float2short100(float value) {
    return FIXED_POINT * value;
}

float short2float100(uint16_t value) {
    return value / 1.0 / FIXED_POINT;
}
