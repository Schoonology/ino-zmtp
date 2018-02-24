#include "zmtp_router.h"
#include "util.h"

ZMTPRouter::ZMTPRouter(uint16_t port) {
  this->client = NULL;

  this->server = new TCPServer(port);
  assert(this->server);

  this->server->begin();
  Serial.printf("Listening on port %u...\n", port);
}

ZMTPRouter::~ZMTPRouter() {
  if (this->client) {
    delete this->client;
  }

  delete this->server;
}

void ZMTPRouter::update() {
  if (this->client) {
    this->client->update();
    return;
  }

  TCPClient client = this->server->available();
  if (client.connected()) {
    this->client = new ZMTPClient(ROUTER, client);
    Serial.println("Client acquired.");
    this->print();
  }
}

bool ZMTPRouter::send(ZMTPFrame *frame) {
  this->update();

  if (!this->client) {
    return false;
  }

  return this->client->send(frame);
}

ZMTPFrame *ZMTPRouter::recv() {
  this->update();

  if (!this->client) {
    return false;
  }

  return this->client->recv();
}

void ZMTPRouter::print() {
  if (!this->client) {
    return;
  }

  this->client->print();
}
