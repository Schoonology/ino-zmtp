#include "zmtp_frame.h"
#include "util.h"

#define ZMTP_FRAMING_OCTETS 2

// Create a new ZMTP frame with the specified size. If data is provided,
// it will be copied into the frame. The provided flags will be set
// and sent along with the frame.
ZMTPFrame::ZMTPFrame(const uint8_t *data, uint8_t size,
                     zmtp_frame_flags_t flags) {
  if (data) {
    this->buffer = (uint8_t *)malloc(size + ZMTP_FRAMING_OCTETS);
    assert(this->buffer);

    this->buffer[0] = flags;
    this->buffer[1] = size;

    memcpy(this->buffer + ZMTP_FRAMING_OCTETS, data, size);
  } else {
    this->buffer = NULL;
  }
}

// Create a new ZMTP frame from the specified, null-terminated string. The
// provided flags will be set and sent along with the frame.
ZMTPFrame::ZMTPFrame(const char *str, zmtp_frame_flags_t flags)
    : ZMTPFrame((const uint8_t *)str, strlen(str), flags) {}

// Create a new ZMTP frame from the specified, null-terminated string. The
// provided flags will be set and sent along with the frame.
ZMTPFrame::ZMTPFrame(const uint8_t *wireData)
    : ZMTPFrame(wireData + 2, wireData[1], (zmtp_frame_flags_t)wireData[0]) {}

// Destroy a frame.
ZMTPFrame::~ZMTPFrame() {
  if (this->buffer) {
    free(this->buffer);
  }
}

// Return the flags associated with this frame, e.g. ZMTP_FRAME_MORE.
zmtp_frame_flags_t ZMTPFrame::flags() {
  if (this->buffer) {
    return (zmtp_frame_flags_t)this->buffer[0];
  } else {
    return ZMTP_FRAME_NONE;
  }
}

// Return the size, in bytes, of the frame.
size_t ZMTPFrame::size() {
  if (this->buffer) {
    return this->buffer[1];
  } else {
    return 0;
  }
}

// Return the size, in bytes, of the frame, _including_ framing bytes.
size_t ZMTPFrame::wireSize() { return this->size() + ZMTP_FRAMING_OCTETS; }

// Return a pointer to the data in the frame.
const uint8_t *ZMTPFrame::data() {
  return this->wireData() + ZMTP_FRAMING_OCTETS;
}

// Return a pointer to the data in the frame, _including_ framing bytes.
const uint8_t *ZMTPFrame::wireData() { return this->buffer; }

// Dump internal state for debugging.
void ZMTPFrame::print() {
  if (this->buffer) {
    zmtp_debug_dump(this->buffer, ZMTP_FRAMING_OCTETS);
    zmtp_debug_dump(this->buffer + ZMTP_FRAMING_OCTETS, this->buffer[1]);
  } else {
    zmtp_debug_dump("Empty frame");
  }
}
