#include "hal_encoder.h"
#include "app/config.h"

static encoder_health_t g_health = { .ok = true, .fault_flags = 0, .last_update_us = 0 };
static encoder_snapshot_t g_snap = {0};

bool hal_encoder_pio_dma_init(void) {
  // TODO: init PIO, state machines, DMA ping-pong
  g_health.ok = true;
  return true;
}

bool hal_encoder_pio_dma_get_snapshot(encoder_snapshot_t* out) {
  // TODO: return latest DMA snapshot
  if (!out) return false;
  *out = g_snap;
  return true;
}

encoder_health_t hal_encoder_pio_dma_health(void) {
  return g_health;
}
