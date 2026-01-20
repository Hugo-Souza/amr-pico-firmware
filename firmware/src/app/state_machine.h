#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
  ST_BOOT = 0,
  ST_IDLE,
  ST_RUN,
  ST_FAULT,
  ST_ESTOP
} system_state_t;

void sm_init(void);
system_state_t sm_get(void);

void sm_request_start(void);
void sm_request_stop(void);

void sm_raise_fault(uint32_t fault_flags);
void sm_clear_fault(void);

void sm_set_estop(bool active);
uint32_t sm_fault_flags(void);
