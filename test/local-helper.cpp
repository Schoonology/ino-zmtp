#ifndef PARTICLE // Only build locally.

#include "assert.h"
#include "czmq.h"

class SocketGuard {
public:
  SocketGuard(zsock_t *socket) : socket(socket) {}
  ~SocketGuard() { zsock_destroy(&socket); }
  zsock_t *socket;
};

const uint8_t *runDealerTest(const char *endpoint) {
  printf("\n");
  printf("DEALER tests:\n");
  printf("=============\n");

  zsock_t *router = zsock_new_router(endpoint);
  assert(router);
  SocketGuard _(router);

  //
  // Receive connection from peer's DEALER
  //
  printf("Client should connect and send a greeting: ");
  zmsg_t *greeting = zmsg_recv(router);
  printf("✔\n");

  //
  // Identity frame
  //
  printf("Client should respond with a single frame: ");
  size_t msg_size = zmsg_size(greeting);
  if (msg_size != 2) {
    printf("✘ — Invalid number of frames (%lu)\n", msg_size);
    return NULL;
  }
  printf("✔\n");

  printf("Client should provide an Identity: ");
  char *identity = zmsg_popstr(greeting);
  if (!streq(identity, "Photon")) {
    printf("✘ — Invalid Identity (%s)\n", identity);
    return NULL;
  }
  printf("✔\n");

  //
  // Message frames
  //
  printf("Client should provide a 5-octet message: ");
  char *message = zmsg_popstr(greeting);
  if (!streq(message, "Hello")) {
    printf("✘ — Invalid message (%s)\n", message);
    return NULL;
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
    return NULL;
  }
  printf("✔\n");
  return zframe_data(frame);
}

bool runRouterTest(const char *endpoint) {
  printf("\n");
  printf("ROUTER tests:\n");
  printf("=============\n");

  //
  // Connect to peer's ROUTER at the specified address
  //
  zsock_t *first = zsock_new(ZMQ_DEALER);
  assert(first);
  SocketGuard _1(first);

  zsock_set_identity(first, "Helper");
  assert(zsock_connect(first, "%s", endpoint) == 0);

  //
  // Connect a second socket
  //
  zsock_t *second = zsock_new(ZMQ_DEALER);
  assert(second);
  SocketGuard _2(second);

  zsock_set_identity(second, "Another");
  assert(zsock_connect(second, "%s", endpoint) == 0);

  printf("---\n");

  //
  // Send a pair of requests
  //
  zmsg_t *question = zmsg_new();
  assert(question);
  assert(zmsg_addstr(question, "Answer?") == 0);
  assert(zmsg_send(&question, first) == 0);

  printf("====\n");

  zmsg_t *ping = zmsg_new();
  assert(ping);
  assert(zmsg_addstr(ping, "ping") == 0);
  assert(zmsg_send(&ping, second) == 0);

  //
  // Get a response (first)
  //
  printf("Client should respond with a single frame: ");
  zmsg_t *answer = zmsg_recv(first);
  size_t answer_size = zmsg_size(answer);
  if (answer_size != 1) {
    printf("✘ — Invalid number of frames (%lu)\n", answer_size);
    return false;
  }
  printf("✔\n");

  printf("Client should respond with a 2-octet answer: ");
  char *answer_str = zmsg_popstr(answer);
  if (!streq(answer_str, "42")) {
    printf("✘ — Invalid answer (%s)\n", answer_str);
    return false;
  }
  printf("✔\n");

  //
  // Get a response (second)
  //
  printf("Client should respond with a single frame: ");
  zmsg_t *pong = zmsg_recv(second);
  size_t pong_size = zmsg_size(pong);
  if (pong_size != 1) {
    printf("✘ — Invalid number of frames (%lu)\n", pong_size);
    return false;
  }
  printf("✔\n");

  printf("Client should respond with a 2-octet answer: ");
  char *pong_str = zmsg_popstr(pong);
  if (!streq(pong_str, "pong")) {
    printf("✘ — Invalid message (%s)\n", pong_str);
    return false;
  }
  printf("✔\n");

  return true;
}

/**
 * Run a peer through the test scenario.
 *
 * NOTE: We're playing fast and loose with memory here; our assumption is
 * that this will be run from an OS that will reap the memory after the
 * process exits, so we're not freeing the memory manually. This may be
 * fixed in a future refactor.
 */
int main(int argc, char const *argv[]) {
  const uint8_t *peerAddress = runDealerTest("tcp://0.0.0.0:1234");
  if (!peerAddress) {
    return 1;
  }

  char peerEndpoint[50];
  sprintf(peerEndpoint, "tcp://%i.%i.%i.%i:1235", peerAddress[0],
          peerAddress[1], peerAddress[2], peerAddress[3]);

  printf("\nPeer endpoint: %s\n", peerEndpoint);

  if (!runRouterTest(peerEndpoint)) {
    return 1;
  }

  printf("\nTearing down.\n");
  return 0;
}

#endif
