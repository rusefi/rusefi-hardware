#ifndef BOARD_INIT_HPP
#define BOARD_INIT_HPP

namespace board {

// Initializes board-level clocks and GPIO to safe defaults based on the schematic.
// Must be called after HAL_Init() and before any driver init.
void init();

} // namespace board

#endif // BOARD_INIT_HPP

