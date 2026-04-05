

[User Documentation](https://github.com/rusefi/rusefi/wiki/GDI4)

https://rusefi.com/forum/viewtopic.php?f=4&t=1337


![x](https://github.com/rusefi/rusefi/wiki/Hardware/GDI/rusefi-gdi4-rev-a.jpg)

## JLCPCB assembly

**DNP** means “do not place”: JLC will not install that whole group of parts. You solder them yourself later, or you fix the order before manufacturing so those spots get real parts from the JLC library. The DNP groups are listed in **`gerber/rev. c/GDI-4ch.csv`** — resistors **R1, R37, R44, R55, R80, R81, R82, R83, R87, R89, R90, R121** and **R5, R28, R30, R32, R117, R118, R119, R120**.

**If JLC is assembling the board for you:** In the online assembly step, check that you actually selected a part for every line you care about. Parts people often forget to turn on: diodes **D5 and D6** (JLC part number **C8678**), and the **0805 jumper pads** that share one line in the parts list (often **C17477** in **`gerber/rev. d/bom.csv`**). File names and lists change between revisions—use the **`gerber/`** folder that matches the board files you ordered.

## HOWTO Update Firmware

UART route https://www.adafruit.com/product/954

See also https://github.com/rusefi/rusefi/blob/master/firmware/controllers/lua/examples/gdi4-communication.lua
