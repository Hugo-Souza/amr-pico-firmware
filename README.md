# AMR Pico Firmware (RP2040)

Firmware modular para AMR usando:
- Raspberry Pi Pico (RP2040) dual-core
- FreeRTOS
- micro-ROS (USB CDC) para integração com ROS 2 / Nav2
- HAL multi-backend:
  - Encoder: PIO+DMA ou LS7366R(SPI)
  - Motor: Sabertooth (packetized serial) ou BTS7960 (PWM/DIR)

Docs:
- docs/ARCHITECTURE.md
- docs/TEST_PLAN.md
