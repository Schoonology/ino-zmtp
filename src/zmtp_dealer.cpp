#include "zmtp_dealer.h"
#include "util.h"

ZMTPDealer::ZMTPDealer(uint8_t *addr, uint16_t port) {
  this->client = new ZMTPClient(DEALER, addr, port);
  assert(this->client);
}

ZMTPDealer::~ZMTPDealer() { delete this->client; }

void ZMTPDealer::setIdentity(uint8_t *data, size_t size) {
  this->client->setIdentity(data, size);
}

void ZMTPDealer::update() { this->client->update(); }

bool ZMTPDealer::send(ZMTPFrame *frame) { return this->client->send(frame); }

ZMTPFrame *ZMTPDealer::recv() { return this->client->recv(); }

void ZMTPDealer::print() { this->client->print(); }
