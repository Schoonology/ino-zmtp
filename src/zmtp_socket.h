#ifndef __ZMTP_SOCKET_H_INCLUDED__
#define __ZMTP_SOCKET_H_INCLUDED__

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#elif defined(SPARK)
  #include "application.h"
#else
  #error Only ARDUINO or SPARK supported.
#endif

typedef enum {
  DEALER,
} zmtp_socket_type_t;

typedef struct _zmtp_socket_t zmtp_socket_t;

zmtp_socket_t *zmtp_socket_new (zmtp_socket_type_t type);

void zmtp_socket_destroy (zmtp_socket_t **self_p);

bool zmtp_socket_connect (zmtp_socket_t *self, uint8_t *addr, uint16_t port);

void zmtp_socket_update (zmtp_socket_t *self);

void zmtp_socket_dump (zmtp_socket_t *self);

#endif
