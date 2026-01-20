#include "config.h"
#include "state_machine.h"
#include "hal/hal_trace_pins.h"
#include "hal/hal_encoder.h"
#include "hal/hal_motor.h"
#include "comm/microros_node.h"

int main(void) {
  // TODO: pico-sdk init (stdio_init_all, clocks, etc.)
  hal_trace_init();
  sm_init();

  // Init hardware layers
  (void)hal_encoder_init();
  (void)hal_motor_init();
  hal_motor_enable(false);

  // Init comms
  (void)microros_init();

  // BOOT -> IDLE
  sm_clear_fault();

  while (1) {
    // Placeholder loop: depois entra FreeRTOS + dual-core tasks.
    // Mantemos micro-ROS "vivo" mesmo no esqueleto.
    microros_spin_some();
  }
  return 0;
}
