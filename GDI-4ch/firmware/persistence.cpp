#include "ch.h"
#include "hal.h"

#include "persistence.h"

#define MFS_RECORD_ID     1

static const MFSConfig mfscfg_1k = {
  .flashp           = (BaseFlash *)&EFLD1,
  .erased           = 0xFFFFFFFFU,
// 1k page * 1 sector = 1024
  .bank_size        = 1024U,
  .bank0_start      = 62U,
  .bank0_sectors    = 1U,
  .bank1_start      = 63U,
  .bank1_sectors    = 1U
};

static const MFSConfig mfscfg_2k = {
    .flashp           = (BaseFlash *)&EFLD1,
    .erased           = 0xFFFFFFFFU,
    /* 256K flash device with 2K pages
     * use last 8 pages for settings
     * one bank is 8K */
// 2k page * 4 sectors = 8096
    .bank_size        = 8096U,
    .bank0_start      = 120U,
    .bank0_sectors    = 4U,
    .bank1_start      = 124U,
    .bank1_sectors    = 4U
};

static MFSDriver mfs1;
uint8_t mfs_buffer[512];

/**
 * @return true if mfsStart is well
 */
mfs_error_t InitFlash() {
  eflStart(&EFLD1, NULL);
  mfsObjectInit(&mfs1);
  mfs_error_t err;
#define FLASH_SIZE_IN_K_ADDRESS     0x1FFFF7E0
    int flashSize = (*(uint16_t*)FLASH_SIZE_IN_K_ADDRESS);
  int flashPageSize = (flashSize > 128) ? 2048 : 1024;
  if (flashSize > 128) {
    err = mfsStart(&mfs1, &mfscfg_2k);
  } else {
    err = mfsStart(&mfs1, &mfscfg_1k);
  }
  return err;
}

static bool isMfsOkIsh(mfs_error_t state) {
    return state == MFS_NO_ERROR || state == MFS_WARN_REPAIR || state == MFS_WARN_GC;
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
    if (!isMfsOkIsh(flashState) || size != GetConfigurationSize() || !configuration.IsValid()) {
        /* load defaults */
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
