#pragma once
#include <stdint.h>

typedef enum {
  TRACE_LOOP_BEGIN = 0,
  TRACE_LOOP_END,
  TRACE_CMD_RX,
  TRACE_ODOM_PUB,
  TRACE_STATE_CHANGE,
  TRACE_FAULT,
} trace_event_t;

void hal_trace_init(void);
void hal_trace_pulse(trace_event_t ev);
