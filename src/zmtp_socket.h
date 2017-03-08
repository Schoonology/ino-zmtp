#ifndef __ZMTP_SOCKET_H_INCLUDED__
#define __ZMTP_SOCKET_H_INCLUDED__

#include "arch.h"
#include "zmtp_frame.h"

typedef enum {
  DEALER,
} zmtp_socket_type_t;

typedef struct _zmtp_socket_t zmtp_socket_t;

zmtp_socket_t *zmtp_socket_new (zmtp_socket_type_t type);

void zmtp_socket_destroy (zmtp_socket_t **self_p);

void zmtp_socket_uuid (zmtp_socket_t *self, uint8_t *uuid);

bool zmtp_socket_ready (zmtp_socket_t *self);

bool zmtp_socket_connect (zmtp_socket_t *self, uint8_t *addr, uint16_t port);

void zmtp_socket_update (zmtp_socket_t *self);

void zmtp_socket_send (zmtp_socket_t *self, zmtp_frame_t *frame);

void zmtp_socket_dump (zmtp_socket_t *self);

#endif
