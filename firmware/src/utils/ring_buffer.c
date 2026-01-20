#include "ring_buffer.h"

void rb_init(ring_buffer_t* rb, uint8_t* storage, uint32_t cap) {
  rb->buf = storage;
  rb->cap = cap;
  rb->head = rb->tail = 0;
}

static uint32_t next_i(uint32_t i, uint32_t cap) { return (i + 1u) % cap; }

bool rb_push(ring_buffer_t* rb, const uint8_t* data, uint32_t len) {
  // Stub simples: não implementa ainda (evita bloquear)
  (void)rb; (void)data; (void)len;
  return false;
}

uint32_t rb_pop(ring_buffer_t* rb, uint8_t* out, uint32_t max_len) {
  (void)rb; (void)out; (void)max_len;
  return 0;
}
