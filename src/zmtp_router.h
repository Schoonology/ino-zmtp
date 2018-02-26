#ifndef __ZMTP_ROUTER_H_INCLUDED__
#define __ZMTP_ROUTER_H_INCLUDED__

#include "arch.h"
#include "zmtp_client.h"
#include "zmtp_socket.h"

/**
 * A duplex ROUTER socket. ROUTERs are server sockets that are connected
 * to by DEALERs.
 */
class ZMTPRouter : public ZMTPSocket {
public:
  // Create a new ROUTER socket. This socket will not be able to send
  // or receive frames until it is connected to.
  ZMTPRouter(uint16_t port);

  // Destroy a socket.
  ~ZMTPRouter();

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
  // TODO(schoon) - For server sockets, use a vector of clients.
  ZMTPClient *client;
  bool needsIdentityPrelude;
  bool awaitingIdentityPrelude;

  TCPServer *server;
};

#endif
