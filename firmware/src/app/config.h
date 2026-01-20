#pragma once
#include <stdint.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
// Profile headers must be injected by CMake
// -----------------------------------------------------------------------------
#ifndef ROBOT_PROFILE_HEADER
#error "ROBOT_PROFILE_HEADER not defined (set via CMake)"
#endif
#ifndef ENCODER_PROFILE_HEADER
#error "ENCODER_PROFILE_HEADER not defined (set via CMake)"
#endif
#ifndef MOTOR_PROFILE_HEADER
#error "MOTOR_PROFILE_HEADER not defined (set via CMake)"
#endif

#include ROBOT_PROFILE_HEADER
#include ENCODER_PROFILE_HEADER
#include MOTOR_PROFILE_HEADER

// -----------------------------------------------------------------------------
// Defaults (if robot profile doesn't define)
// -----------------------------------------------------------------------------
#ifndef CONTROL_HZ
#define CONTROL_HZ 100
#endif
#ifndef ODOM_PUB_HZ
#define ODOM_PUB_HZ 50
#endif
#ifndef STATUS_PUB_HZ
#define STATUS_PUB_HZ 20
#endif
#ifndef CMD_TIMEOUT_MS
#define CMD_TIMEOUT_MS 200
#endif
#ifndef VEL_IIR_ALPHA
#define VEL_IIR_ALPHA 0.25f
#endif
#ifndef MOTOR_DEADBAND
#define MOTOR_DEADBAND 0.02f
#endif
#ifndef ODOM_FRAME
#define ODOM_FRAME "odom"
#endif
#ifndef BASE_FRAME
#define BASE_FRAME "base_link"
#endif

// -----------------------------------------------------------------------------
// Required robot params
// -----------------------------------------------------------------------------
#ifndef WHEEL_RADIUS_M
#error "WHEEL_RADIUS_M missing in robot profile"
#endif
#ifndef WHEEL_TRACK_M
#error "WHEEL_TRACK_M missing in robot profile"
#endif
#ifndef ENC_PPR_MOTOR_REV
#error "ENC_PPR_MOTOR_REV missing in robot profile"
#endif
#ifndef GEAR_RATIO_NUM
#error "GEAR_RATIO_NUM missing in robot profile"
#endif
#ifndef GEAR_RATIO_DEN
#error "GEAR_RATIO_DEN missing in robot profile"
#endif
#if (GEAR_RATIO_DEN == 0)
#error "GEAR_RATIO_DEN cannot be 0"
#endif

// -----------------------------------------------------------------------------
// Encoder backend selection (must be exactly one)
// -----------------------------------------------------------------------------
#if !defined(ENCODER_BACKEND_PIO_DMA) && !defined(ENCODER_BACKEND_LS7366R_SPI)
#error "No encoder backend selected in encoder profile"
#endif
#if defined(ENCODER_BACKEND_PIO_DMA) && defined(ENCODER_BACKEND_LS7366R_SPI)
#error "Multiple encoder backends selected"
#endif

// -----------------------------------------------------------------------------
// Motor backend selection (must be exactly one)
// -----------------------------------------------------------------------------
#if !defined(MOTOR_BACKEND_SABERTOOTH) && !defined(MOTOR_BACKEND_BTS7960)
#error "No motor backend selected in motor profile"
#endif
#if defined(MOTOR_BACKEND_SABERTOOTH) && defined(MOTOR_BACKEND_BTS7960)
#error "Multiple motor backends selected"
#endif

// -----------------------------------------------------------------------------
// Derived constants
// -----------------------------------------------------------------------------
#define CONTROL_TS_S     (1.0f / (float)CONTROL_HZ)
#define ODOM_PUB_TS_S    (1.0f / (float)ODOM_PUB_HZ)
#define STATUS_PUB_TS_S  (1.0f / (float)STATUS_PUB_HZ)

#ifndef AMR_PI_F
#define AMR_PI_F 3.14159265358979323846f
#endif

// Counts per wheel revolution
#define COUNTS_PER_WHEEL_REV ((int64_t)ENC_PPR_MOTOR_REV * (int64_t)GEAR_RATIO_NUM / (int64_t)GEAR_RATIO_DEN)
#define RAD_PER_COUNT ((2.0f * AMR_PI_F) / (float)COUNTS_PER_WHEEL_REV)

#ifdef BUILD_INFO_STR
#define AMR_BUILD_INFO BUILD_INFO_STR
#else
#define AMR_BUILD_INFO "unknown"
#endif
