#include "util.h"

void __debug_dump (const char *label, const uint8_t *buffer, uint8_t len) {
  assert (label);
  assert (buffer);

  Serial.print (label);
  Serial.print (" = ");
  for (uint8_t i = 0; i < len; i++) {
    Serial.printf ("%02X", buffer[i]);
  }
  Serial.println ();
}

void zero_bytes(uint8_t *buffer, uint8_t offset, uint8_t length) {
  assert (buffer);

  for (uint8_t i = offset; i < (offset + length); i++) {
    buffer[i] = 0;
  }
}

uint8_t *uuid_new () {
  uint8_t * uuid = (uint8_t *) malloc (16);
  assert (uuid);

  for (uint8_t i = 0; i < 16; ++i) {
    uuid[i] = random (256);
  }

  return uuid;
}

bool uuid_equal (uint8_t *first, uint8_t *second) {
  assert (first);
  assert (second);

  return !memcmp (first, second, 16);
}
