#include "zmtp_frame.h"
#include "util.h"

struct _zmtp_frame_t {
  uint8_t *buffer;
};

zmtp_frame_t *zmtp_frame_new (const uint8_t *data, uint8_t size) {
  zmtp_frame_t *self = (zmtp_frame_t *) malloc (sizeof (zmtp_frame_t));
  assert (self);

  if (data) {
    self->buffer = (uint8_t *) malloc (size + 2);
    assert (self->buffer);

    self->buffer[0] = 0;
    self->buffer[1] = size;

    memcpy (self->buffer + 2, data, size);
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
  return self->buffer[1] + 2;
}

uint8_t *zmtp_frame_bytes (zmtp_frame_t *self) {
  assert (self);

  return self->buffer;
}

void zmtp_frame_dump (zmtp_frame_t *self) {
  assert (self);

  if (self->buffer) {
    debug_dump (self->buffer[0]);
    debug_dump (self->buffer[1]);
    debug_dump (self->buffer + 2, self->buffer[1]);
  } else {
    debug_dump ("Empty frame");
  }
}
