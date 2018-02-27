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

class ZMTPFrame {
public:
  // Create a new ZMTP frame with the specified size. If data is provided,
  // it will be copied into the frame. The provided flags will be set
  // and sent along with the frame.
  ZMTPFrame(const uint8_t *data, uint8_t size, zmtp_frame_flags_t flags);

  // Create a new ZMTP frame with the specified size. If data is provided,
  // it will be copied into the frame. Flags will be set to ZMTP_FRAME_NONE.
  ZMTPFrame(const uint8_t *data, uint8_t size);

  // Create a new ZMTP frame from the specified, null-terminated string. The
  // provided flags will be set and sent along with the frame.
  ZMTPFrame(const char *str, zmtp_frame_flags_t flags);

  // Create a new ZMTP frame from the specified, null-terminated string.
  // Flags will be set to ZMTP_FRAME_NONE.
  ZMTPFrame(const char *str);

  // Create a new ZMTP frame from the specified wire data.
  ZMTPFrame(const uint8_t *wireData);

  // Destroy a frame.
  ~ZMTPFrame();

  // Compares the contents of one frame to another, returning 0 when equal.
  // NOTE: This does not compare flags.
  int compare(ZMTPFrame *other);

  // Return the flags associated with this frame, e.g. ZMTP_FRAME_MORE.
  zmtp_frame_flags_t flags();

  // Return the size, in bytes, of the frame.
  size_t size();

  // Return the size, in bytes, of the frame, _including_ framing bytes.
  size_t wireSize();

  // Return a pointer to the data in the frame.
  const uint8_t *data();

  // Return a pointer to the data in the frame, _including_ framing bytes.
  const uint8_t *wireData();

  // Dump internal state for debugging.
  void print();

private:
  uint8_t *buffer;
};

#endif
