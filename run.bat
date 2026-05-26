@echo off
REM AI Automation Script

if "%1"=="build" goto build
if "%1"=="flash" goto flash
if "%1"=="test" goto test
if "%1"=="clean" goto clean

echo =========================================
echo Usage: run [command]
echo Commands:
echo   build   - Cross-compile MCU firmware
echo   flash   - Flash firmware using DAPLink
echo   test    - Run unit tests on PC host
echo   clean   - Clean build directories
echo =========================================
exit /b 1

:build
echo [AI-Script] Starting cross-compilation...
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE="gcc-arm-none-eabi.cmake" -G Ninja -B build
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build build
exit /b %errorlevel%

:flash
echo [AI-Script] Flashing firmware to MSPM0...
openocd -f interface/cmsis-dap.cfg -c "adapter speed 2000" -f target/ti_mspm0.cfg -c "program build/MSPM0G3507_FreeRTOS.elf verify reset exit"
exit /b %errorlevel%

:test
echo [AI-Script] Running host unit tests...
cmake -G Ninja -B build_host
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build build_host
if %errorlevel% neq 0 exit /b %errorlevel%
.\build_host\run_tests.exe
exit /b %errorlevel%

:clean
echo [AI-Script] Cleaning build directories...
if exist build rmdir /s /q build
if exist build_host rmdir /s /q build_host
echo Done.
exit /b 0
