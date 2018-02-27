#include "zmtp_router.h"
#include "util.h"

ZMTPRouter::ZMTPRouter(uint16_t port) {
  this->activeIncoming = NULL;
  this->activeOutgoing = NULL;

  this->server = new TCPServer(port);
  assert(this->server);

  this->server->begin();
  ZMTPLog.trace("Listening on port %u...", port);
}

ZMTPRouter::~ZMTPRouter() {
  while (this->clients.size()) {
    delete this->clients.takeFirst();
  }

  delete this->server;
}

void ZMTPRouter::update() {
  for (int i = 0; i < this->clients.size(); ++i) {
    this->clients[i]->update();
  }

  TCPClient client = this->server->available();
  if (client.connected()) {
    this->clients.append(new ZMTPClient(ROUTER, client));
    ZMTPLog.trace("Client acquired.");
    this->print();
  }
}

bool ZMTPRouter::send(ZMTPFrame *frame) {
  this->update();

  // As long as we have an active client, send this frame there.
  if (this->activeOutgoing) {
    ZMTPClient *client = this->activeOutgoing;

    if (frame->flags() == ZMTP_FRAME_NONE) {
      this->activeOutgoing = NULL;
    }

    return client->send(frame);
  }

  // If we don't have an active client, then we're between messages. This
  // should be interpreted as an identity frame.
  // TODO(schoon) - Throw an error if the identity frame is missing
  // the ZMTP_FRAME_MORE flag?
  for (int i = 0; i < this->clients.size(); ++i) {
    if (this->clients[i]->getIdentity()->compare(frame) == 0) {
      this->activeOutgoing = this->clients[i];
      return true;
    }
  }

  // No active client and no matching client—bail.
  return false;
}

ZMTPFrame *ZMTPRouter::recv() {
  this->update();

  // As long as we have an active client, receive from that client.
  if (this->activeIncoming) {
    ZMTPFrame *frame = this->activeIncoming->recv();
    if (!frame) {
      return NULL;
    }

    if (frame->flags() == ZMTP_FRAME_NONE) {
      this->activeIncoming = NULL;
    }

    return frame;
  }

  // If we don't have an active client, then we're between messages.
  // We need to scan all attached clients, set the active client, but
  // return the identity frame instead.
  for (int i = 0; i < this->clients.size(); ++i) {
    if (this->clients[i]->peek()) {
      this->activeIncoming = this->clients[i];
      return this->clients[i]->getIdentity();
    }
  }

  // No active client and no client with pending messages—bail.
  return NULL;
}

void ZMTPRouter::print() {
  for (int i = 0; i < this->clients.size(); ++i) {
    this->clients[i]->print();
  }
}
