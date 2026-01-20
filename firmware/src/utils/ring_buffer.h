#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint8_t* buf;
  uint32_t cap;
  volatile uint32_t head;
  volatile uint32_t tail;
} ring_buffer_t;

void rb_init(ring_buffer_t* rb, uint8_t* storage, uint32_t cap);
bool rb_push(ring_buffer_t* rb, const uint8_t* data, uint32_t len);
uint32_t rb_pop(ring_buffer_t* rb, uint8_t* out, uint32_t max_len);
