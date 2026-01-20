#include "hal_encoder.h"
#include "app/config.h"

static encoder_health_t g_health = { .ok = true, .fault_flags = 0, .last_update_us = 0 };
static encoder_snapshot_t g_snap = {0};

bool hal_encoder_ls7366r_spi_init(void) {
  // TODO: init SPI + chip selects + configure LS7366R
  g_health.ok = true;
  return true;
}

bool hal_encoder_ls7366r_spi_get_snapshot(encoder_snapshot_t* out) {
  // TODO: read counters via SPI and fill snapshot
  if (!out) return false;
  *out = g_snap;
  return true;
}

encoder_health_t hal_encoder_ls7366r_spi_health(void) {
  return g_health;
}
