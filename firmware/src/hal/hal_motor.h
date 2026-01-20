#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  bool ok;
  uint32_t fault_flags;
} motor_health_t;

bool hal_motor_init(void);
void hal_motor_enable(bool en);
void hal_motor_set_u(float u_l, float u_r);
motor_health_t hal_motor_health(void);
