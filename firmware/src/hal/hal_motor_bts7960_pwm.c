#include "hal_motor.h"
#include "app/config.h"

static motor_health_t g_health = { .ok = true, .fault_flags = 0 };
static bool g_enabled = false;

bool hal_motor_bts7960_init(void) {
  // TODO: init PWM slices, DIR GPIOs
  g_enabled = false;
  return true;
}

void hal_motor_bts7960_enable(bool en) {
  g_enabled = en;
  // TODO: zero PWM when disabling
}

static float clamp1(float x) {
  if (x > 1.0f) return 1.0f;
  if (x < -1.0f) return -1.0f;
  return x;
}

void hal_motor_bts7960_set_u(float u_l, float u_r) {
  if (!g_enabled) return;
  u_l = clamp1(u_l);
  u_r = clamp1(u_r);
  // TODO: apply deadband, set dir pins, set PWM duty
  (void)u_l; (void)u_r;
}

motor_health_t hal_motor_bts7960_health(void) {
  return g_health;
}
