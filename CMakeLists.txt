cmake_minimum_required(VERSION 3.16)

# -------- Pico SDK import --------
include(pico_sdk_import.cmake)

project(amr_pico_firmware_standalone C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Init Pico SDK (sets up toolchain, stdlib, hardware libs)
pico_sdk_init()

# -------- Profile selection (forwarded to firmware/) --------
set(ROBOT_PROFILE   "amr_diff_generic"         CACHE STRING "profiles/robot/<name>.h")
set(ENCODER_PROFILE "encoder_pio_dma"          CACHE STRING "profiles/encoder/<name>.h")
set(MOTOR_PROFILE   "motor_sabertooth_packet"  CACHE STRING "profiles/motor/<name>.h")

message(STATUS "Standalone ROBOT_PROFILE   = ${ROBOT_PROFILE}")
message(STATUS "Standalone ENCODER_PROFILE = ${ENCODER_PROFILE}")
message(STATUS "Standalone MOTOR_PROFILE   = ${MOTOR_PROFILE}")

# -------- Build firmware library (your code) --------
add_subdirectory(firmware)

# -------- Create executable that links your fw lib --------
add_executable(amr_fw_standalone
  firmware/src/app/main.c
)

# IMPORTANT:
# Your firmware library already includes main.c in the lib target in your current CMakeLists.txt.
# For standalone, you should NOT compile main.c twice.
# Choose ONE strategy:
#   A) main.c belongs to the executable (recommended)
#   B) main.c belongs to the library (not recommended)
#
# If you keep main.c in fw library, remove it from this add_executable() list.
# If you prefer main.c in the executable (recommended), remove main.c from firmware/CMakeLists.txt sources.

target_link_libraries(amr_fw_standalone
  fw
  pico_stdlib
  hardware_gpio
  hardware_uart
  hardware_spi
  hardware_pwm
  hardware_dma
  hardware_pio
  hardware_timer
)

# Enable USB CDC stdio (good for logs) + optionally UART
pico_enable_stdio_usb(amr_fw_standalone 1)
pico_enable_stdio_uart(amr_fw_standalone 0)

# Generate .uf2/.bin/.hex
pico_add_extra_outputs(amr_fw_standalone)

# Pass profile choices down to fw (overrides defaults in firmware/CMakeLists.txt)
# (Only needed if you don't want to set these when configuring fw directly.)
set_target_properties(fw PROPERTIES
  COMPILE_DEFINITIONS
    "ROBOT_PROFILE_HEADER=\"profiles/robot/${ROBOT_PROFILE}.h\";ENCODER_PROFILE_HEADER=\"profiles/encoder/${ENCODER_PROFILE}.h\";MOTOR_PROFILE_HEADER=\"profiles/motor/${MOTOR_PROFILE}.h\""
)
