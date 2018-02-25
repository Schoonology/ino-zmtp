#ifndef __ZMTP_CLIENT_H_INCLUDED__
#define __ZMTP_CLIENT_H_INCLUDED__

#include "arch.h"
#include "zmtp_frame.h"

typedef enum {
  DEALER,
  ROUTER,
} zmtp_client_type_t;

// Internal states.
typedef enum {
  CONNECTING, // Connection pending
  INIT,       // Connection established
  SIG_WAIT,   // ZMTP signature sent
  SIG_ACK,    // ZMTP signature acknowledged
  VER_ACK,    // ZMTP version acknowledged
  GRT_ACK,    // ZMTP greeting acknowledged
  READY_WAIT, // ZMTP READY command sent
  READY,      // ZMTP READY command acknowledged
  _ERROR,     // Connection or ZMTP handshake failed
} zmtp_client_state_t;

/**
 * A duplex ZMTP socket over TCP.
 */
class ZMTPClient {
public:
  ZMTPClient(zmtp_client_type_t type, uint8_t *addr, uint16_t port);
  ZMTPClient(zmtp_client_type_t type, TCPClient client);

  ~ZMTPClient();

  // Retrieves the identity of this or the attached peer, represented
  // as a Frame.
  const ZMTPFrame *getIdentity();

  // Assigns the identity of this or the attached peer as A Frame.
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
  void sendGreeting();
  void sendHandshake();

  TCPClient client;
  uint8_t *frameBuffer;
  ZMTPFrame *identity;
  IPAddress peerAddress;
  uint16_t peerPort;
  Vector<ZMTPFrame *> receivedFrames;
  zmtp_client_state_t state;
  zmtp_client_type_t type;
};

#endif
