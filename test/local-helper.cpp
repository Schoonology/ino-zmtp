#ifndef PARTICLE // Only build locally.

#include "assert.h"
#include "czmq.h"

int main(int argc, char const *argv[])
{
  zsock_t *socket = zsock_new_router ("tcp://192.168.29.198:1234");
  assert (socket);

  printf ("Endpoint: %s\n", zsock_endpoint (socket));

  zmsg_t *greeting = zmsg_recv (socket);
  zmsg_print (greeting);

  zmsg_t *message = zmsg_recv (socket);
  zmsg_print (message);

  printf ("Tearing down.\n");
  zsock_destroy (&socket);
  return 0;
}

#endif
