#pragma once
#define ENCODER_BACKEND_LS7366R_SPI 1

// SPI (exemplo — ajuste)
#define SPI_ID        0
#define SPI_SCK_GPIO  18
#define SPI_MOSI_GPIO 19
#define SPI_MISO_GPIO 16
#define CS_L_GPIO     17
#define CS_R_GPIO     20

#define LS7366R_SPI_HZ 2000000

#define ENC_L_INVERT  0
#define ENC_R_INVERT  0
