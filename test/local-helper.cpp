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
  printf ("Client should provide a 16-octet UUID: ");
  size_t msg_size = zmsg_size (greeting);
  if (msg_size != 2) {
    printf("✘ — Invalid number of frames (%lu)\n", msg_size);
    BAIL();
  }

  char * uuid = zmsg_popstr (greeting);
  // TODO(schoon) - Remove ZRE-assuming 17th byte.
  // TODO(schoon) - Test and allow arbitrary identities.
  if (!streq (uuid, "\x01\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10")) {
    printf("✘ — Invalid UUID (%s)\n", uuid);
    BAIL();
  }
  printf ("✔\n");

  //
  // Message frame
  //
  printf ("Client should provide a 5-octet message: ");
  char * message = zmsg_popstr (greeting);
  if (!streq (message, "Hello")) {
    printf("✘ — Invalid message (%s)\n", message);
    BAIL();
  }
  printf ("✔\n");

  printf ("Tearing down.\n");
  zsock_destroy (&socket);
  return 0;
}

#endif
