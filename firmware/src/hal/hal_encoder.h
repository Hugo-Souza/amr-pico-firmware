#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  int32_t count_l;
  int32_t count_r;
  uint32_t t_us;
  uint32_t flags;
} encoder_snapshot_t;

typedef struct {
  bool ok;
  uint32_t fault_flags;
  uint32_t last_update_us;
} encoder_health_t;

bool hal_encoder_init(void);
bool hal_encoder_get_snapshot(encoder_snapshot_t* out);
encoder_health_t hal_encoder_health(void);
