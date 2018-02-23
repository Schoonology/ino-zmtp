#ifndef PARTICLE // Only build locally.

#include "assert.h"
#include "czmq.h"

/**
 * Run a peer through the test scenario.
 *
 * NOTE: We're playing fast and loose with memory here; our assumption is
 * that this will be run from an OS that will reap the memory after the
 * process exits, so we're not freeing the memory manually. This may be
 * fixed in a future refactor.
 */
int main(int argc, char const *argv[]) {
  zsock_t *router = zsock_new_router("tcp://0.0.0.0:1234");
  assert(router);

  //
  // Receive connection from peer's DEALER
  //
  printf("Client should connect and send a greeting: ");
  zmsg_t *greeting = zmsg_recv(router);
  zmsg_t *duplicate = zmsg_dup(greeting);
  printf("✔\n");

  //
  // Identity frame
  //
  printf("Client should provide an Identity: ");
  size_t msg_size = zmsg_size(greeting);
  if (msg_size != 2) {
    printf("✘ — Invalid number of frames (%lu)\n", msg_size);
    return 1;
  }

  char *uuid = zmsg_popstr(greeting);
  if (!streq(uuid, "Photon")) {
    printf("✘ — Invalid Identity (%s)\n", uuid);
    return 1;
  }
  printf("✔\n");

  //
  // Message frames
  //
  printf("Client should provide a 5-octet message: ");
  char *message = zmsg_popstr(greeting);
  if (!streq(message, "Hello")) {
    printf("✘ — Invalid message (%s)\n", message);
    return 1;
  }
  printf("✔\n");

  //
  // Receive IP address from peer
  //
  printf("Client should send IP address: ");
  zmsg_t *address = zmsg_recv(router);
  zmsg_pop(address);
  zframe_t *frame = zmsg_pop(address);
  size_t address_size = zframe_size(frame);
  if (address_size != 4) {
    printf("✘ — Invalid address size (%lu)\n", address_size);
    return 1;
  }
  uint8_t *bytes = zframe_data(frame);
  char address_string[50];
  sprintf(address_string, "tcp://%i.%i.%i.%i:1235", bytes[0], bytes[1],
          bytes[2], bytes[3]);
  printf("Address: %s\n", address_string);
  printf("✔\n");

  //
  // Connect to peer's ROUTER at the specified address
  //
  zsock_t *dealer = zsock_new_dealer(address_string);
  assert(dealer);

  //
  // Send a request
  //
  zmsg_t *question = zmsg_new();
  assert(question);
  assert(zmsg_addstr(question, "Answer?") == 0);
  assert(zmsg_send(&question, dealer) == 0);

  //
  // Get a response
  //
  zmsg_t *answer = zmsg_recv(dealer);
  printf("Client should respond with a single frame: ");
  size_t answer_size = zmsg_size(answer);
  if (answer_size != 1) {
    printf("✘ — Invalid number of frames (%lu)\n", answer_size);
    return 1;
  }

  printf("Client should respond with a 2-octet answer: ");
  char *answer_str = zmsg_popstr(answer);
  if (!streq(answer_str, "42")) {
    printf("✘ — Invalid answer (%s)\n", answer_str);
    return 1;
  }
  printf("✔\n");

  //
  // Cleanup
  //
  printf("Tearing down.\n");
  zsock_destroy(&router);
  zsock_destroy(&dealer);
  return 0;
}

#endif
