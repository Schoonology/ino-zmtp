#include "zmtp_uuid.h"
#include "util.h"

struct _zmtp_uuid_t {
  uint8_t buffer[ZMTP_UUID_LENGTH];
};

zmtp_uuid_t *zmtp_uuid_new () {
  zmtp_uuid_t * self = (zmtp_uuid_t *) malloc (sizeof (zmtp_uuid_t));
  assert (self);

  for (uint8_t i = 0; i < ZMTP_UUID_LENGTH; ++i) {
    self->buffer[i] = random (256);
  }

  return self;
}

void zmtp_uuid_destroy (zmtp_uuid_t **self_p) {
  assert (self_p);

  if (*self_p) {
    zmtp_uuid_t *self = *self_p;
    free (self);
    *self_p = NULL;
  }
}

bool zmtp_uuid_equal (zmtp_uuid_t *first, zmtp_uuid_t *second) {
  assert (first);
  assert (second);

  return !memcmp (first, second, ZMTP_UUID_LENGTH);
}

uint8_t *zmtp_uuid_bytes (zmtp_uuid_t *self) {
  assert (self);
  return self->buffer;
}

void zmtp_uuid_dump (zmtp_uuid_t *self) {
  assert (self);
  zmtp_debug_dump (self->buffer, ZMTP_UUID_LENGTH);
}
