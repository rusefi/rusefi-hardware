@echo off
rem Same as flash.bat but using STM32CubeProgrammer CLI instead of the legacy ST-LINK Utility.
rem connect: SWD, mode=UR = connect under reset (st-link_cli "ur"); -w write, -v verify, -rst reset & run

set CLI=STM32_Programmer_CLI
where %CLI% >nul 2>nul || set CLI="C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"

%CLI% -c port=SWD mode=UR -w build\dinputs.hex -v -rst
