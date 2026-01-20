# -----------------------------------------------------------------------------
# Makefile - AMR Pico Firmware (standalone build)
# -----------------------------------------------------------------------------
# Usage examples:
#   make
#   make ROBOT_PROFILE=amr_diff_generic ENCODER_PROFILE=encoder_pio_dma MOTOR_PROFILE=motor_sabertooth_packet
#   make clean
#   make flash UF2_PATH=build/amr_fw_standalone.uf2
#
# Notes:
# - "flash" via picotool requires picotool built with USB support + device visible in Linux/WSL.
# - You can always flash by copying UF2 to the RPI-RP2 drive (BOOTSEL).
# -----------------------------------------------------------------------------

# Build directory
BUILD_DIR ?= build

# CMake generator
GENERATOR ?= "Unix Makefiles"

# Profiles (separated)
ROBOT_PROFILE   ?= amr_diff_generic
ENCODER_PROFILE ?= encoder_pio_dma
MOTOR_PROFILE   ?= motor_sabertooth_packet

# Executable name (must match add_executable in root CMakeLists.txt)
TARGET ?= amr_fw_standalone

# Optional: additional CMake args
CMAKE_EXTRA ?=

# UF2 output (default location for Pico builds)
UF2_PATH ?= $(BUILD_DIR)/$(TARGET).uf2

# If you flash by copying UF2 to mounted drive, set:
#   UF2_MOUNT=/mnt/<drive>
# Example on Linux: /media/<user>/RPI-RP2
# Example on WSL (Windows mount): /mnt/d (if the drive letter is D:)
UF2_MOUNT ?=

# Tools
CMAKE ?= cmake
NPROC ?= $(shell nproc 2>/dev/null || echo 4)
PICOTOOL ?= picotool

.PHONY: all configure build uf2 clean distclean \
        flash flash-uf2 copy-uf2 info

all: build

info:
	@echo "BUILD_DIR       = $(BUILD_DIR)"
	@echo "TARGET          = $(TARGET)"
	@echo "ROBOT_PROFILE   = $(ROBOT_PROFILE)"
	@echo "ENCODER_PROFILE = $(ENCODER_PROFILE)"
	@echo "MOTOR_PROFILE   = $(MOTOR_PROFILE)"
	@echo "UF2_PATH        = $(UF2_PATH)"
	@echo "UF2_MOUNT       = $(UF2_MOUNT)"

configure:
	@mkdir -p $(BUILD_DIR)
	@$(CMAKE) -S . -B $(BUILD_DIR) -G $(GENERATOR) \
		-DROBOT_PROFILE=$(ROBOT_PROFILE) \
		-DENCODER_PROFILE=$(ENCODER_PROFILE) \
		-DMOTOR_PROFILE=$(MOTOR_PROFILE) \
		$(CMAKE_EXTRA)

build: configure
	@$(CMAKE) --build $(BUILD_DIR) -j$(NPROC)

uf2: build
	@if [ -f "$(UF2_PATH)" ]; then \
		echo "UF2 ready: $(UF2_PATH)"; \
	else \
		echo "UF2 not found at: $(UF2_PATH)"; \
		echo "Check TARGET name or build output."; \
		exit 1; \
	fi

clean:
	@if [ -d "$(BUILD_DIR)" ]; then \
		$(CMAKE) --build $(BUILD_DIR) --target clean || true; \
	fi

distclean:
	@rm -rf $(BUILD_DIR)

# -----------------------------------------------------------------------------
# Flash options
# -----------------------------------------------------------------------------

# 1) Flash using picotool (requires USB support and device visible)
flash: uf2
	@echo "Flashing via picotool: $(UF2_PATH)"
	@$(PICOTOOL) load -f $(UF2_PATH)

# 2) Copy UF2 to mounted RPI-RP2 drive (BOOTSEL mode)
copy-uf2: uf2
	@if [ -z "$(UF2_MOUNT)" ]; then \
		echo "UF2_MOUNT not set. Example:"; \
		echo "  make copy-uf2 UF2_MOUNT=/media/$$USER/RPI-RP2"; \
		echo "  make copy-uf2 UF2_MOUNT=/mnt/d   (WSL: if RPI-RP2 is drive D:)"; \
		exit 1; \
	fi
	@if [ ! -d "$(UF2_MOUNT)" ]; then \
		echo "UF2_MOUNT directory not found: $(UF2_MOUNT)"; \
		exit 1; \
	fi
	@echo "Copying UF2 to: $(UF2_MOUNT)"
	@cp -f $(UF2_PATH) $(UF2_MOUNT)/
	@echo "Done. Pico should reboot automatically."

# Alias
flash-uf2: copy-uf2
