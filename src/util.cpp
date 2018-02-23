#include "util.h"

void __zmtp_debug_dump(const char *label, const uint8_t *buffer, uint8_t len) {
  assert(label);
  assert(buffer);

  Serial.print(label);
  Serial.print(" = ");
  for (uint8_t i = 0; i < len; i++) {
    Serial.printf("%02X", buffer[i]);
  }
  Serial.println();
}
