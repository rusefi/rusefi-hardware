#ifndef HIGH_SIDE_CONTROL_HPP
#define HIGH_SIDE_CONTROL_HPP

#include <cstdint>

// High-side control helper for the gate-driver HIN nets (HS_VBATx / HS_VBOOSTx).
//
// By default the firmware keeps all high-side enables LOW for safety. This module allows
// enabling them in a conservative, safety-gated way via build-time configuration.
namespace high_side {

void init();

// Call periodically (e.g. from a task loop).
// If `systemHealthy` is false, all high-side enables are forced LOW immediately.
void update(bool systemHealthy);

} // namespace high_side

#endif // HIGH_SIDE_CONTROL_HPP

