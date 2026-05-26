# AI Automation Script (Cross-Platform Makefile)
# 为 Linux/macOS 环境提供与 run.bat 完全一致的快捷指令体验

.PHONY: all build flash test clean

all:
	@echo "========================================="
	@echo "Usage: make [command]"
	@echo "Commands:"
	@echo "  build   - Cross-compile MCU firmware"
	@echo "  flash   - Flash firmware using DAPLink"
	@echo "  test    - Run unit tests on PC host"
	@echo "  clean   - Clean build directories"
	@echo "========================================="

build:
	@echo "[AI-Script] Starting cross-compilation..."
	cmake -DCMAKE_TOOLCHAIN_FILE=gcc-arm-none-eabi.cmake -G Ninja -B build
	cmake --build build

flash:
	@echo "[AI-Script] Flashing firmware to MSPM0..."
	openocd -f interface/cmsis-dap.cfg -c "adapter speed 2000" -f target/ti_mspm0.cfg -c "program build/MSPM0G3507_FreeRTOS.elf verify reset exit"

test:
	@echo "[AI-Script] Running host unit tests..."
	cmake -G Ninja -B build_host
	cmake --build build_host
	./build_host/run_tests

clean:
	@echo "[AI-Script] Cleaning build directories..."
	rm -rf build build_host
	@echo "Done."
