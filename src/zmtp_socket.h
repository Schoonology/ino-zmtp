/**
 * A duplex ZMTP socket.
 *
 * Currently, only DEALER and ROUTER sockets are supported.
 */

#ifndef __ZMTP_SOCKET_H_INCLUDED__
#define __ZMTP_SOCKET_H_INCLUDED__

#include "arch.h"
#include "zmtp_frame.h"
#include "zmtp_uuid.h"

// Socket types supported by ZMTP sockets.
typedef enum {
  DEALER,
  ROUTER,
} zmtp_socket_type_t;

// Internal states.
typedef enum {
  INIT = 0,   // New connection
  CONNECTING, // Trying to connect
  LISTENING,  // Listening for incoming connections
  SIG_WAIT,   // ZMTP signature sent
  SIG_ACK,    // ZMTP signature acknowledged
  VER_ACK,    // ZMTP version acknowledged
  GRT_ACK,    // ZMTP greeting acknowledged
  READY_WAIT, // ZMTP READY command sent
  READY,      // ZMTP READY command acknowledged
  _ERROR,     // Connection or ZMTP handshake failed
} zmtp_socket_state_t;

class ZMTPSocket {
public:
  // Create a new socket of the specified type. This socket will not
  // be able to send or receive frames until it is connected.
  ZMTPSocket(zmtp_socket_type_t type);

  // Destroy a socket.
  ~ZMTPSocket();

  // Set the "identity" associated with this socket. The identity may
  // be any bytes in length, and will be copied into the internal state.
  void setIdentity(uint8_t *data, size_t size);

  // Return true if the socket is connected and the ZMTP handshake
  // has completed, and false otherwise. Has no side effects; call
  // `update` to update internal state.
  bool ready();

  // Connect the ZMTP socket to the TCP port at the specified port
  // and address. The address should be 4 bytes in length.
  bool connect(uint8_t *addr, uint16_t port);

  // Bind the ZMTP socket to the specified TCP port, listening for
  // incoming connections.
  void bind(uint16_t port);

  // Update the internal ZMTP state. This should be called periodically
  // to read received data.
  void update();

  // Send the frame through this socket.
  void send(zmtp_frame_t *frame);

  // Receives the next frame from the socket, or NULL if no frame is
  // available.
  zmtp_frame_t *recv();

  // Dump internal state for debugging.
  void print();

private:
  void sendGreeting();
  void sendHandshake();

  uint8_t peer_address[4];
  uint16_t peer_port;
  zmtp_socket_type_t type;
  zmtp_socket_state_t state;
  uint8_t *identity;
  size_t identity_size;
  // TODO(schoon) - For server sockets, use a vector of clients.
  TCPClient socket;
  TCPServer *server;
  Vector<zmtp_frame_t *> frame_queue;
};

#endif
