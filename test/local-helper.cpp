#ifndef PARTICLE // Only build locally.

#include "assert.h"
#include "czmq.h"

#define BAIL() \
  printf("Message:\n"); \
  zmsg_print (duplicate); \
  zmsg_destroy (&greeting); \
  zmsg_destroy (&duplicate); \
  zsock_destroy (&socket); \
  return 1;

int main(int argc, char const *argv[])
{
  zsock_t *socket = zsock_new_router ("tcp://0.0.0.0:1234");
  assert (socket);

  //
  // Connection
  //
  printf ("Client should connect and send a greeting: ");
  zmsg_t *greeting = zmsg_recv (socket);
  zmsg_t *duplicate = zmsg_dup (greeting);
  printf ("✔\n");

  //
  // Identity frame
  //
  printf ("Client should provide an Identity: ");
  size_t msg_size = zmsg_size (greeting);
  if (msg_size != 2) {
    printf("✘ — Invalid number of frames (%lu)\n", msg_size);
    BAIL();
  }

  char * uuid = zmsg_popstr (greeting);
  if (!streq (uuid, "Photon")) {
    printf("✘ — Invalid Identity (%s)\n", uuid);
    BAIL();
  }
  printf ("✔\n");
  free (uuid);

  //
  // Message frames
  //
  printf ("Client should provide a 5-octet message: ");
  char * message = zmsg_popstr (greeting);
  if (!streq (message, "Hello")) {
    printf("✘ — Invalid message (%s)\n", message);
    BAIL();
  }
  printf ("✔\n");
  free (message);

  //
  // Cleanup
  //
  printf ("Tearing down.\n");
  zmsg_destroy (&greeting);
  zmsg_destroy (&duplicate);
  zsock_destroy (&socket);
  return 0;
}

#endif
