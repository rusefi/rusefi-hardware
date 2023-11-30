#include "global.h"

#include <rusefi/rusefi_time_wraparound.h>
#include <rusefi/rusefi_time_math.h>

static WrapAround62 timeNt;

/**
 * 64-bit counter CPU/timer cycles since MCU reset
 */
efitick_t getTimeNowNt() {
	return timeNt.update(getTimeNowLowerNt());
}

/**
 * 64-bit result would not overflow, but that's complex stuff for our 32-bit MCU
 */
efitimeus_t getTimeNowUs() {
#if ENABLE_PERF_TRACE
	ScopePerf perf(PE::GetTimeNowUs);
#endif // ENABLE_PERF_TRACE
	return NT2US(getTimeNowNt());
}
