/**
 * A single ZMTP frame, including both the message data and framing.
 *
 * Currently, only "short" frames (those with length 255 or less)
 * are supported.
 */

#ifndef __ZMTP_FRAME_H_INCLUDED__
#define __ZMTP_FRAME_H_INCLUDED__

#include "arch.h"

// Flags supported by ZMTP frames.
typedef enum {
  ZMTP_FRAME_NONE = 0,
  ZMTP_FRAME_MORE = 1,
} zmtp_frame_flags_t;

// Opaque type.
typedef struct _zmtp_frame_t zmtp_frame_t;

// Create a new ZMTP frame with the specified size. If data is provided,
// it will be copied into the new frame.
//
// No flags are set. (Same as ZMTP_FRAME_NONE.)
zmtp_frame_t *zmtp_frame_new (const uint8_t *data, uint8_t size);

// Create a new ZMTP frame with the specified size. If data is provided,
// it will be copied into the frame. The provided flags will be set
// and sent along with the frame.
zmtp_frame_t *zmtp_frame_new (const uint8_t *data, uint8_t size, zmtp_frame_flags_t flags);

// Destroy a frame.
void zmtp_frame_destroy (zmtp_frame_t **self_p);

// Updates the flags on a ZMTP frame.
void zmtp_frame_flags (zmtp_frame_t *self, zmtp_frame_flags_t flags);

// Return the size, in bytes, of the frame. This is larger than the
// original size parameter in `zmtp_frame_new`, as this size includes
// framing.
uint8_t zmtp_frame_size (zmtp_frame_t *self);

// Return a pointer to the frame, including framing (flags and size).
uint8_t *zmtp_frame_bytes (zmtp_frame_t *self);

// Dump internal state.
void zmtp_frame_dump (zmtp_frame_t *self);

#endif
