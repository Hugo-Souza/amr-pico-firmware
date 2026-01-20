#include "state_machine.h"

static volatile system_state_t g_state = ST_BOOT;
static volatile uint32_t g_fault_flags = 0;
static volatile bool g_estop = false;

void sm_init(void) { g_state = ST_BOOT; g_fault_flags = 0; g_estop = false; }
system_state_t sm_get(void) { return g_state; }
uint32_t sm_fault_flags(void) { return g_fault_flags; }

void sm_request_start(void) {
  if (!g_estop && g_state == ST_IDLE) g_state = ST_RUN;
}
void sm_request_stop(void) {
  if (g_state == ST_RUN) g_state = ST_IDLE;
}

void sm_raise_fault(uint32_t fault_flags) {
  g_fault_flags |= fault_flags;
  g_state = ST_FAULT;
}

void sm_clear_fault(void) {
  if (!g_estop) { g_fault_flags = 0; g_state = ST_IDLE; }
}

void sm_set_estop(bool active) {
  g_estop = active;
  g_state = active ? ST_ESTOP : ST_IDLE;
}
