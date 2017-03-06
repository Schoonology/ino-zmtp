#include "zmtp_frame.h"
#include "util.h"

struct _zmtp_frame_t {
  uint8_t flags;
  uint8_t size;
  uint8_t *buffer;
};

zmtp_frame_t *zmtp_frame_new (const uint8_t *data, uint8_t size) {
  zmtp_frame_t *self = (zmtp_frame_t *) malloc (sizeof (zmtp_frame_t));
  assert (self);

  self->flags = 0;

  if (data) {
    self->size = size;
    self->buffer = (uint8_t *) malloc (size);
    assert (self->buffer);

    memcpy (self->buffer, data, size);
  } else {
    self->size = 0;
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

void zmtp_frame_dump (zmtp_frame_t *self) {
  assert (self);

  debug_dump (self->flags);
  debug_dump (self->size);
  debug_dump (self->buffer, self->size);
}
