@echo off
setlocal enabledelayedexpansion

set "THISDIR=%~dp0"
rem Remove trailing backslash
if "%THISDIR:~-1%"=="\" set "THISDIR=%THISDIR:~0,-1%"

set "DFU_PROGRAMMER=%THISDIR%\dfu-programmer.exe"

set "FW=%~1"
set "KM=%~2"

goto :start
rem define subroutines first

:abort
set "MSG=%~1"
echo !MSG:"=!
pause
exit 2

:get-bootloader
"%DFU_PROGRAMMER%" atmega32u2 get bootloader-version 2>NUL
exit /b %errorlevel%

:wait-bootloader
:wait-again
timeout /t 2 /nobreak >NUL
call :get-bootloader
if errorlevel 1 (
    goto wait-again
)
exit /b 0


:start
if "%~2"=="" (
    call :abort "USAGE: %~nx0 path\to\firmware.hex path\to\keymap.hex"
)
if not exist "%FW%" (
    call :abort "ERROR: firmware file '%FW%' not found"
)
if not exist "%KM%" (
    call :abort "ERROR: keymap file '%KM%' not found"
)

"%DFU_PROGRAMMER%" --version
if errorlevel 1 (
    call :abort "ERROR: dfu-programmer tool not found"
)

echo Preparing to flash firmware for %~n2 ...
echo.
echo Please plug in keyboard to be flashed, and put into bootloader/flash mode.
echo Waiting for keyboard bootloader ...
call :wait-bootloader

echo.
echo *** Wiping existing flash memory and EEPROM...
echo.
"%DFU_PROGRAMMER%" atmega32u2 erase --force
"%DFU_PROGRAMMER%" atmega32u2 flash --force --suppress-validation --eeprom "%THISDIR%\reset.eep"
"%DFU_PROGRAMMER%" atmega32u2 flash --force "%THISDIR%\eeprom_eraser.hex"

echo.
echo *** Resetting controller for main flash...
echo.
"%DFU_PROGRAMMER%" atmega32u2 reset
call :wait-bootloader

echo.
echo *** Flashing firmware %FW% ...
echo.
"%DFU_PROGRAMMER%" atmega32u2 erase --force
"%DFU_PROGRAMMER%" atmega32u2 flash --force "%FW%"

echo.
echo *** Flashing keymap %KM% ...
echo.
"%DFU_PROGRAMMER%" atmega32u2 flash --force --eeprom "%KM%"

echo.
echo *** Rebooting keyboard controller...
"%DFU_PROGRAMMER%" atmega32u2 reset

echo.
echo ...done!
pause
exit 0
