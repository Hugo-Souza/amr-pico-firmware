cmake_minimum_required(VERSION 3.16)
project(amr_pico_firmware C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# ---- Profile selection ----

set(ENCODER_PROFILE "encoder_pio_dma"          CACHE STRING "profiles/encoder/<name>.h")
set(MOTOR_PROFILE   "motor_sabertooth_packet"  CACHE STRING "profiles/motor/<name>.h")
set(ROBOT_PROFILE   "amr_diff_generic"         CACHE STRING "profiles/robot/<name>.h")

message(STATUS "ROBOT_PROFILE   = ${ROBOT_PROFILE}")
message(STATUS "ENCODER_PROFILE = ${ENCODER_PROFILE}")
message(STATUS "MOTOR_PROFILE   = ${MOTOR_PROFILE}")

add_library(fw STATIC
  #src/app/main.c
  src/app/state_machine.c

  src/hal/hal_trace_pins.c

  # HAL wrappers
  src/hal/hal_encoder.c
  src/hal/hal_motor.c

  # Backends (selected below)
  # src/hal/hal_encoder_pio_dma.c
  # src/hal/hal_encoder_ls7366r_spi.c
  # src/hal/hal_motor_sabertooth_packet.c
  # src/hal/hal_motor_bts7960_pwm.c

  src/comm/microros_node.c
  src/utils/ring_buffer.c
)

target_include_directories(fw PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/src
  #${CMAKE_CURRENT_LIST_DIR}/profiles
  ${CMAKE_CURRENT_LIST_DIR}
)

# Inject profile headers into config.h
target_compile_definitions(fw PUBLIC
  ROBOT_PROFILE_HEADER="profiles/robot/${ROBOT_PROFILE}.h"
  ENCODER_PROFILE_HEADER="profiles/encoder/${ENCODER_PROFILE}.h"
  MOTOR_PROFILE_HEADER="profiles/motor/${MOTOR_PROFILE}.h"
)

string(TIMESTAMP BUILD_TS "%Y-%m-%d_%H:%M:%S")
target_compile_definitions(fw PUBLIC BUILD_INFO_STR="\"${PROJECT_NAME}:${BUILD_TS}\"")

# ---- Backend source selection by profile name ----
if(ENCODER_PROFILE STREQUAL "encoder_pio_dma")
  target_sources(fw PRIVATE src/hal/hal_encoder_pio_dma.c)
elseif(ENCODER_PROFILE STREQUAL "encoder_ls7366r_spi")
  target_sources(fw PRIVATE src/hal/hal_encoder_ls7366r_spi.c)
else()
  message(FATAL_ERROR "Unknown ENCODER_PROFILE: ${ENCODER_PROFILE}")
endif()

if(MOTOR_PROFILE STREQUAL "motor_sabertooth_packet")
  target_sources(fw PRIVATE src/hal/hal_motor_sabertooth_packet.c)
elseif(MOTOR_PROFILE STREQUAL "motor_bts7960_pwm")
  target_sources(fw PRIVATE src/hal/hal_motor_bts7960_pwm.c)
else()
  message(FATAL_ERROR "Unknown MOTOR_PROFILE: ${MOTOR_PROFILE}")
endif()

# NOTE: pico-sdk + micro_ros_setup toolchain/link libraries are provided by the
# parent build. This library is intended to be consumed by that build.
