/**
 * A duplex ZMTP socket.
 *
 * Currently, only DEALER sockets are supported.
 */

#ifndef __ZMTP_SOCKET_H_INCLUDED__
#define __ZMTP_SOCKET_H_INCLUDED__

#include "arch.h"
#include "zmtp_frame.h"

// Socket types supported by ZMTP sockets.
typedef enum {
  DEALER,
} zmtp_socket_type_t;

// Opaque type.
typedef struct _zmtp_socket_t zmtp_socket_t;

// Create a new socket of the specified type. This socket will not
// be able to send or receive frames until it is connected.
zmtp_socket_t *zmtp_socket_new (zmtp_socket_type_t type);

// Destroy a socket.
void zmtp_socket_destroy (zmtp_socket_t **self_p);

// Set the UUID "identity" associated with this socket. The UUID
// should be 16 bytes in length, which will be copied into the
// internal state.
void zmtp_socket_uuid (zmtp_socket_t *self, uint8_t *uuid);

// Return true if the socket is connected and the ZMTP handshake
// has completed, and false otherwise.
bool zmtp_socket_ready (zmtp_socket_t *self);

// Connect the ZMTP socket to the TCP port at the specified port
// and address. The address should be 4 bytes in length.
bool zmtp_socket_connect (zmtp_socket_t *self, uint8_t *addr, uint16_t port);

// Update the internal ZMTP state. This should be called periodically
// to read received data.
void zmtp_socket_update (zmtp_socket_t *self);

// Send the frame through this socket.
void zmtp_socket_send (zmtp_socket_t *self, zmtp_frame_t *frame);

// Dump internal state.
void zmtp_socket_dump (zmtp_socket_t *self);

#endif
