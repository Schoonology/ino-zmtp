#ifndef PARTICLE // Only build locally.

#include "assert.h"
#include "czmq.h"

int main(int argc, char const *argv[])
{
  zsock_t *socket = zsock_new_dealer ("tcp://192.168.29.198:1234");
  assert (socket);

  printf("Connected. Sending...\n");

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
