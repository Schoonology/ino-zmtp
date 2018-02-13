#ifndef PARTICLE // Only build locally.

#include "assert.h"
#include "czmq.h"

int main(int argc, char const *argv[])
{
  zsock_t *socket = zsock_new (ZMQ_DEALER);
  assert (socket);

  zsock_set_identity (socket, "\x01\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10");
  zsock_connect (socket, "tcp://0.0.0.0:1234");
  printf ("Connected. Sending...\n");

  zmsg_t *greeting = zmsg_new ();
  zmsg_print (greeting);
  zmsg_pushstr (greeting, "Hello");
  zmsg_print (greeting);
  zmsg_send (&greeting, socket);

  sleep (1);

  printf ("Tearing down.\n");
  zsock_destroy (&socket);
  return 0;
}

#endif
