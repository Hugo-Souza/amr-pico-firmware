#include "hal_motor.h"
#include "app/config.h"

#ifdef MOTOR_BACKEND_SABERTOOTH
bool hal_motor_sabertooth_init(void);
void hal_motor_sabertooth_enable(bool en);
void hal_motor_sabertooth_set_u(float u_l, float u_r);
motor_health_t hal_motor_sabertooth_health(void);

bool hal_motor_init(void) { return hal_motor_sabertooth_init(); }
void hal_motor_enable(bool en) { hal_motor_sabertooth_enable(en); }
void hal_motor_set_u(float u_l, float u_r) { hal_motor_sabertooth_set_u(u_l, u_r); }
motor_health_t hal_motor_health(void) { return hal_motor_sabertooth_health(); }

#elif defined(MOTOR_BACKEND_BTS7960)
bool hal_motor_bts7960_init(void);
void hal_motor_bts7960_enable(bool en);
void hal_motor_bts7960_set_u(float u_l, float u_r);
motor_health_t hal_motor_bts7960_health(void);

bool hal_motor_init(void) { return hal_motor_bts7960_init(); }
void hal_motor_enable(bool en) { hal_motor_bts7960_enable(en); }
void hal_motor_set_u(float u_l, float u_r) { hal_motor_bts7960_set_u(u_l, u_r); }
motor_health_t hal_motor_health(void) { return hal_motor_bts7960_health(); }

#else
#error "No motor backend compiled"
#endif
