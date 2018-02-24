#ifndef __ZMTP_DEALER_H_INCLUDED__
#define __ZMTP_DEALER_H_INCLUDED__

#include "arch.h"
#include "zmtp_client.h"
#include "zmtp_socket.h"

/**
 * A duplex DEALER socket. DEALERs are client sockets connected to ROUTERs.
 */
class ZMTPDealer : public ZMTPSocket {
public:
  // Create a new DEALER socket. This socket will not be able to send
  // or receive frames until it has completely connected.
  ZMTPDealer(uint8_t *addr, uint16_t port);

  // Destroy a socket.
  ~ZMTPDealer();

  // Set the "identity" associated with this socket. The identity may
  // be any bytes in length, and will be copied into the internal state.
  void setIdentity(uint8_t *data, size_t size);

  // Update the internal ZMTP state. This should be called periodically
  // to read received data.
  void update();

  // Send the frame through this socket. Returns true if sending was
  // succesful, false otherwise.
  bool send(ZMTPFrame *frame);

  // Returns the next frame already received through the socket, or
  // NULL if no frame is available.
  ZMTPFrame *recv();

  // Dump internal state for debugging.
  void print();

private:
  ZMTPClient *client;
};

#endif
