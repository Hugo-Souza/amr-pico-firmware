#include "hal_motor.h"
#include "app/config.h"

static motor_health_t g_health = { .ok = true, .fault_flags = 0 };
static bool g_enabled = false;

bool hal_motor_sabertooth_init(void) {
  // TODO: init UART, configure packetized mode, set address
  g_enabled = false;
  return true;
}

void hal_motor_sabertooth_enable(bool en) {
  g_enabled = en;
  // TODO: send stop command if disabling
}

static float clamp1(float x) {
  if (x > 1.0f) return 1.0f;
  if (x < -1.0f) return -1.0f;
  return x;
}

void hal_motor_sabertooth_set_u(float u_l, float u_r) {
  if (!g_enabled) return;
  u_l = clamp1(u_l);
  u_r = clamp1(u_r);
  // TODO: map to Sabertooth packetized commands + checksum
  (void)u_l; (void)u_r;
}

motor_health_t hal_motor_sabertooth_health(void) {
  return g_health;
}
