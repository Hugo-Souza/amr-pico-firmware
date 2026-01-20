#include "hal_encoder.h"
#include "app/config.h"

#ifdef ENCODER_BACKEND_PIO_DMA
bool hal_encoder_pio_dma_init(void);
bool hal_encoder_pio_dma_get_snapshot(encoder_snapshot_t* out);
encoder_health_t hal_encoder_pio_dma_health(void);

bool hal_encoder_init(void) { return hal_encoder_pio_dma_init(); }
bool hal_encoder_get_snapshot(encoder_snapshot_t* out) { return hal_encoder_pio_dma_get_snapshot(out); }
encoder_health_t hal_encoder_health(void) { return hal_encoder_pio_dma_health(); }

#elif defined(ENCODER_BACKEND_LS7366R_SPI)
bool hal_encoder_ls7366r_spi_init(void);
bool hal_encoder_ls7366r_spi_get_snapshot(encoder_snapshot_t* out);
encoder_health_t hal_encoder_ls7366r_spi_health(void);

bool hal_encoder_init(void) { return hal_encoder_ls7366r_spi_init(); }
bool hal_encoder_get_snapshot(encoder_snapshot_t* out) { return hal_encoder_ls7366r_spi_get_snapshot(out); }
encoder_health_t hal_encoder_health(void) { return hal_encoder_ls7366r_spi_health(); }

#else
#error "No encoder backend compiled"
#endif
