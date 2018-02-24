#ifndef __ZMTP_SOCKET_H_INCLUDED__
#define __ZMTP_SOCKET_H_INCLUDED__

#include "arch.h"
#include "zmtp_frame.h"

/**
 * Abstract type representing a duplex ZMTP socket.
 */
class ZMTPSocket {
public:
  // Update the internal ZMTP state. This should be called periodically
  // to read received data.
  virtual void update() = 0;

  // Send the frame through this socket. Returns true if sending was
  // succesful, false otherwise.
  virtual bool send(ZMTPFrame *frame) = 0;

  // Returns the next frame already received through the socket, or
  // NULL if no frame is available.
  virtual ZMTPFrame *recv() = 0;

  // Dump internal state for debugging.
  virtual void print() = 0;
};

#endif
