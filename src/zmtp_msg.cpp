#include "zmtp_msg.h"
#include "util.h"

struct _zmtp_msg_t {
  zmtp_frame_t **frame_list;
  uint8_t frame_count;
};

zmtp_msg_t *zmtp_msg_new () {
  zmtp_msg_t *self = (zmtp_msg_t *) malloc (sizeof (zmtp_msg_t));
  assert (self);
  return self;
}

void zmtp_msg_destroy (zmtp_msg_t **self_p) {
  assert (self_p);

  if (*self_p) {
    zmtp_msg_t *self = *self_p;

    if (self->frame_list) {
      for (uint8_t i = 0; i < self->frame_count; ++i) {
        zmtp_frame_t *frame = self->frame_list[i];
        zmtp_frame_destroy (&frame);
      }

      free (self->frame_list);
    }

    free(self);
    *self_p = NULL;
  }
}

void zmtp_msg_push (zmtp_msg_t *self, zmtp_frame_t **frame_p) {
  assert (self);
  assert (frame_p);
  assert (*frame_p);

  uint8_t frame_count = self->frame_count + 1;
  zmtp_frame_t **frame_list = (zmtp_frame_t **) malloc (sizeof (zmtp_frame_t *) * frame_count);
  if (self->frame_list) {
    zmtp_frame_flags (self->frame_list[self->frame_count - 1], ZMTP_FRAME_MORE);

    memcpy (frame_list, self->frame_list, self->frame_count);
    free (self->frame_list);
  }
  self->frame_list = frame_list;
  self->frame_count = frame_count;

  self->frame_list[self->frame_count - 1] = *frame_p;
  *frame_p = NULL;
}

zmtp_frame_t *zmtp_msg_pop (zmtp_msg_t *self) {
  assert (self);

  if (!self->frame_list) {
    return NULL;
  }

  uint8_t frame_count = self->frame_count - 1;
  zmtp_frame_t *frame = self->frame_list[self->frame_count - 1];

  if (frame_count > 0) {
    zmtp_frame_t **frame_list = (zmtp_frame_t **) malloc (sizeof (zmtp_frame_t *) * frame_count);
    memcpy (frame_list, self->frame_list, frame_count);
    free (self->frame_list);
    self->frame_list = frame_list;
    self->frame_count = frame_count;
  } else {
    self->frame_list = NULL;
    self->frame_count = 0;
  }

  return frame;
}

void zmtp_msg_send (zmtp_msg_t **self_p, zmtp_socket_t *socket) {
  assert (self_p);
  assert (socket);

  zmtp_msg_t *self = *self_p;
  assert (self);

  for (uint8_t i = 0; i < self->frame_count; ++i) {
    zmtp_frame_t *frame = self->frame_list[i];
    zmtp_socket_send (socket, frame);
  }

  zmtp_msg_destroy (self_p);
}
