/**
 * A set of ZMTP frames, updating the flags of contained frames
 * (i.e. with ZMTP_FRAME_MORE) as necessary.
 */

#ifndef __ZMTP_MSG_H_INCLUDED__
#define __ZMTP_MSG_H_INCLUDED__

#include "zmtp_frame.h"
#include "zmtp_socket.h"

// Opaque struct.
typedef struct _zmtp_msg_t zmtp_msg_t;

// Create a new, empty ZMTP message.
zmtp_msg_t *zmtp_msg_new();

// Destroy a ZMTP message.
void zmtp_msg_destroy(zmtp_msg_t **self_p);

// Push a frame onto the end of the ZMTP message, taking ownership
// of that frame.
void zmtp_msg_push(zmtp_msg_t *self, zmtp_frame_t **frame_p);

// Pop a frame from the end of the ZMTP message, transferring
// ownership of that frame to the caller.
zmtp_frame_t *zmtp_msg_pop(zmtp_msg_t *self);

// Sends a ZMTP message over a ZMTP socket, consuming the message
// in the process.
void zmtp_msg_send(zmtp_msg_t **self_p, ZMTPSocket &socket);

void zmtp_msg_dump(zmtp_msg_t *self);

#endif
