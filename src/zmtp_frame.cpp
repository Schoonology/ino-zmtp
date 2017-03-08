#include "zmtp_frame.h"
#include "util.h"

#define ZMTP_FRAMING_OCTETS 2

struct _zmtp_frame_t {
  uint8_t *buffer;
};

zmtp_frame_t *zmtp_frame_new (const uint8_t *data, uint8_t size) {
  return zmtp_frame_new (data, size, ZMTP_FRAME_NONE);
}

zmtp_frame_t *zmtp_frame_new (const uint8_t *data, uint8_t size, zmtp_frame_flags_t flags) {
  zmtp_frame_t *self = (zmtp_frame_t *) malloc (sizeof (zmtp_frame_t));
  assert (self);

  if (data) {
    self->buffer = (uint8_t *) malloc (size + ZMTP_FRAMING_OCTETS);
    assert (self->buffer);

    self->buffer[0] = flags;
    self->buffer[1] = size;

    memcpy (self->buffer + ZMTP_FRAMING_OCTETS, data, size);
  } else {
    self->buffer = NULL;
  }

  return self;
}

void zmtp_frame_destroy (zmtp_frame_t **self_p) {
  assert (self_p);

  if (*self_p) {
    zmtp_frame_t *self = *self_p;

    if (self->buffer) {
      free (self->buffer);
    }

    free(self);
    *self_p = NULL;
  }
}

uint8_t zmtp_frame_size (zmtp_frame_t *self) {
  assert (self);

  if (self->buffer) {
    return self->buffer[1] + ZMTP_FRAMING_OCTETS;
  } else {
    return 0;
  }
}

uint8_t *zmtp_frame_bytes (zmtp_frame_t *self) {
  assert (self);

  return self->buffer;
}

void zmtp_frame_dump (zmtp_frame_t *self) {
  assert (self);

  if (self->buffer) {
    debug_dump (self->buffer, ZMTP_FRAMING_OCTETS);
    debug_dump (self->buffer + ZMTP_FRAMING_OCTETS, self->buffer[1]);
  } else {
    debug_dump ("Empty frame");
  }
}
