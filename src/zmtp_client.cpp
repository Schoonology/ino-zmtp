#include "zmtp_client.h"
#include "util.h"

zmtp_client_state_t parseGreeting(zmtp_client_state_t oldState, uint8_t *buffer,
                                  size_t length) {
  assert(buffer);

  zmtp_client_state_t newState = oldState;

  if (newState == SIG_WAIT) {
    Serial.printf("buffer: %x, length: %d\n", buffer, length);

    if (length < 10 || buffer[0] != 0xFF || buffer[9] != 0x7F) {
      return _ERROR;
    }

    newState = SIG_ACK;
    buffer += 10;
    length -= 10;
  }

  if (length == 0) {
    return newState;
  }

  if (newState == SIG_ACK) {
    Serial.printf("buffer: %x, length: %d\n", buffer, length);

    if (length < 2 || buffer[0] != 0x03 || buffer[1] != 0x00) {
      return _ERROR;
    }

    newState = VER_ACK;
    buffer += 2;
    length -= 2;
  }

  if (length == 0) {
    return newState;
  }

  if (newState == VER_ACK) {
    Serial.printf("buffer: %x, length: %d\n", buffer, length);

    if (length < 52 || memcmp(buffer, "NULL", 4)) {
      return _ERROR;
    }

    newState = GRT_ACK;
  }

  return newState;
}

zmtp_client_state_t parseHandshake(uint8_t *buffer, int length) {
  assert(buffer);

  if (length <= 2 || buffer[0] != 0x04) {
    return _ERROR;
  }

  uint8_t command_size = buffer[1];

  if (command_size > length || command_size < 6) {
    return _ERROR;
  }

  if (buffer[2] != 0x05 || !memcpy(buffer + 3, "READY", 5)) {
    return _ERROR;
  }

  return READY;
}

ZMTPClient::ZMTPClient(zmtp_client_type_t type, uint8_t *addr, uint16_t port) {
  this->frameBuffer = new uint8_t[256];
  this->identity = NULL;
  this->identitySize = 0;
  this->peerAddress = IPAddress(addr);
  this->peerPort = port;
  this->state = CONNECTING;
  this->type = type;
}

ZMTPClient::ZMTPClient(zmtp_client_type_t type, TCPClient client) {
  this->client = client;
  this->frameBuffer = new uint8_t[256];
  this->identity = NULL;
  this->identitySize = 0;
  this->state = INIT;
  this->type = type;
}

ZMTPClient::~ZMTPClient() {
  delete this->frameBuffer;

  if (this->identity) {
    delete this->identity;
  }
}

void ZMTPClient::setIdentity(uint8_t *data, size_t size) {
  assert(data);
  assert(size < 256);

  if (this->identity) {
    delete this->identity;
  }

  this->identity = new uint8_t[size];
  memcpy(this->identity, data, size);
  this->identitySize = size;
}

void ZMTPClient::update() {
  Particle.process();

  Serial.println("Attempting to connect.");
  if (this->state == CONNECTING &&
      this->client.connect(this->peerAddress, this->peerPort)) {
    Serial.println("Connected.");
    this->state = INIT;
  }

  // Guard
  if (!this->client.connected()) {
    Serial.println("Not connected.");
    return;
  }

  Serial.println("Moving on...");

  if (this->state == INIT) {
    this->sendGreeting();
  }

  while (this->client.available()) {
    int bytes_read = this->client.read(this->frameBuffer, 256);
    Serial.printf("Bytes read: %d\n", bytes_read);

    Serial.print("Bytes: ");
    zmtp_debug_dump(this->frameBuffer, bytes_read);

    switch (this->state) {
    case INIT:
    case SIG_WAIT:
    case SIG_ACK:
    case VER_ACK:
    case GRT_ACK:
      this->state = parseGreeting(this->state, this->frameBuffer, bytes_read);

      this->print();

      if (this->state == GRT_ACK) {
        Serial.println("Received ZMTP greeting.");
        this->sendHandshake();
      }
      break;

    case READY_WAIT:
      this->state = parseHandshake(this->frameBuffer, bytes_read);
      if (this->state == READY) {
        Serial.println("Received ZMTP handshake.");
      }
      break;

    case READY:
      Serial.println("Message frame.");
      this->receivedFrames.append(new ZMTPFrame(this->frameBuffer));
      break;

    case _ERROR:
    default:
      break;
    }
  }
}

bool ZMTPClient::send(ZMTPFrame *frame) {
  assert(frame);
  Serial.println("Sending: ");
  frame->print();
  this->update();

  if (this->state != READY) {
    return false;
  }

  int written = this->client.write(frame->wireData(), frame->wireSize());
  Serial.printf("Bytes written: %i\n", written);
  this->client.flush();
}

ZMTPFrame *ZMTPClient::recv() {
  this->update();

  if (this->state != READY || !this->receivedFrames.size()) {
    return NULL;
  }

  return this->receivedFrames.takeFirst();
}

void ZMTPClient::print() { zmtp_debug_dump(this->state); }

void ZMTPClient::sendGreeting() {
  Serial.println("Sending greeting...");

  uint8_t greeting[64];
  memset(greeting, 0, 64);

  // signature
  greeting[0] = 0xFF;
  greeting[9] = 0x7F;

  // version
  greeting[10] = 0x03;
  greeting[11] = 0x00;

  // mechanism
  memcpy(greeting + 12, "NULL", 4);

  // as-server
  greeting[32] = this->type == DEALER ? 0x00 : 0x01;

  zmtp_debug_dump(greeting, 64);

  int written = this->client.write(greeting, 64);
  Serial.printf("Bytes written: %i\n", written);
  this->client.flush();

  this->state = SIG_WAIT;
}

void ZMTPClient::sendHandshake() {
  Serial.println("Sending handshake...");

  size_t handshake_size = 43 + identitySize;
  uint8_t *handshake = new uint8_t[handshake_size];
  memset(handshake, 0, handshake_size);

  // command-size
  handshake[0] = 0x04;
  handshake[1] = handshake_size - 2;

  // ready
  handshake[2] = 0x05;
  memcpy(handshake + 3, "READY", 5);

  // metadata
  handshake[8] = 11;
  memcpy(handshake + 9, "Socket-Type", 11);
  handshake[23] = 6;
  if (this->type == DEALER) {
    memcpy(handshake + 24, "DEALER", 6);
  } else {
    memcpy(handshake + 24, "ROUTER", 6);
  }

  handshake[30] = 8;
  memcpy(handshake + 31, "Identity", 8);
  handshake[42] = identitySize;
  memcpy(handshake + 43, identity, identitySize);

  zmtp_debug_dump(handshake, handshake_size);

  int written = this->client.write(handshake, handshake_size);
  Serial.printf("Bytes written: %i\n", written);
  this->client.flush();

  this->state = READY_WAIT;
}
