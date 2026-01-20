#pragma once

// Geometria (ajuste depois com seu robô)
#define WHEEL_RADIUS_M  0.050f
#define WHEEL_TRACK_M   0.300f

// Encoder / mecânica (seu caso)
#define ENC_PPR_MOTOR_REV  1840
#define GEAR_RATIO_NUM     49
#define GEAR_RATIO_DEN     1

// Rates
#define CONTROL_HZ      100
#define ODOM_PUB_HZ     50
#define STATUS_PUB_HZ   20

// Safety
#define CMD_TIMEOUT_MS  200

// Filtro (estimação)
#define VEL_IIR_ALPHA   0.25f

// Frames
#define ODOM_FRAME      "odom"
#define BASE_FRAME      "base_link"
