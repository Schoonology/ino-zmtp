#ifndef __ZMTP_FRAME_H_INCLUDED__
#define __ZMTP_FRAME_H_INCLUDED__

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#elif defined(SPARK)
  #include "application.h"
#else
  #error Only ARDUINO or SPARK supported.
#endif

typedef enum {
  NONE,
  MORE,
} zmtp_frame_flags_t;

typedef struct _zmtp_frame_t zmtp_frame_t;

zmtp_frame_t *zmtp_frame_new (const uint8_t *data, uint8_t size);

zmtp_frame_t *zmtp_frame_new (const uint8_t *data, uint8_t size, zmtp_frame_flags_t flags);

void zmtp_frame_destroy (zmtp_frame_t **self_p);

uint8_t zmtp_frame_size (zmtp_frame_t *self);

uint8_t *zmtp_frame_bytes (zmtp_frame_t *self);

void zmtp_frame_dump (zmtp_frame_t *self);

#endif
