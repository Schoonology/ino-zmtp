#include "zmtp_socket.h"
#include "util.h"

zmtp_socket_state_t parse_greeting (zmtp_socket_state_t old_state, uint8_t *buffer, int length) {
  assert (buffer);

  zmtp_socket_state_t new_state = old_state;

  if (new_state == SIG_WAIT) {
    Serial.printf("buffer: %x, length: %d\n", buffer, length);

    if (length < 10 || buffer[0] != 0xFF || buffer[9] != 0x7F) {
      return _ERROR;
    }

    new_state = SIG_ACK;
    buffer += 10;
    length -= 10;
  }

  if (length == 0) {
    return new_state;
  }

  if (new_state == SIG_ACK) {
    Serial.printf("buffer: %x, length: %d\n", buffer, length);

    if (length < 2 || buffer[0] != 0x03 || buffer[1] != 0x00) {
      return _ERROR;
    }

    new_state = VER_ACK;
    buffer += 2;
    length -= 2;
  }

  if (length == 0) {
    return new_state;
  }

  if (new_state == VER_ACK) {
    Serial.printf("buffer: %x, length: %d\n", buffer, length);

    if (length < 52 || memcmp (buffer, "NULL", 4)) {
      return _ERROR;
    }

    new_state = GRT_ACK;
  }

  return new_state;
}

zmtp_socket_state_t parse_handshake (uint8_t *buffer, int length) {
  assert (buffer);

  if (length <= 2 || buffer[0] != 0x04) {
    return _ERROR;
  }

  uint8_t command_size = buffer[1];

  if (command_size > length || command_size < 6) {
    return _ERROR;
  }

  if (buffer[2] != 0x05 || !memcpy (buffer + 3, "READY", 5)) {
    return _ERROR;
  }

  return READY;
}

ZMTPSocket::ZMTPSocket (zmtp_socket_type_t type) {
  this->type = type;
  this->state = INIT;
  this->identity = NULL;
  this->identity_size = 0;
  this->server = NULL;
}

ZMTPSocket::~ZMTPSocket () {
  delete this->identity;
}

void ZMTPSocket::setIdentity (uint8_t *data, size_t size) {
  assert (this->type == DEALER);
  assert (data);
  assert (size < 256);

  if (this->identity) {
    delete this->identity;
  }

  this->identity = new uint8_t[size];
  memcpy (this->identity, data, size);
  identity_size = size;
}

bool ZMTPSocket::ready () {
  return this->state == READY;
}

bool ZMTPSocket::connect (uint8_t *addr, uint16_t port) {
  assert (this->type == DEALER);
  assert (this->state == INIT || this->state == CONNECTING);
  assert (addr);

  if (addr != this->peer_address) {
    memcpy (this->peer_address, addr, 4);
    this->peer_port = port;
  }

  bool success = this->socket.connect (IPAddress (this->peer_address), port);

  if (success) {
    Serial.println ("Connected.");

    this->sendGreeting ();
  } else {
    Serial.println ("Failed to connect.");

    this->state = CONNECTING;
  }

  this->print ();

  return success;
}

void ZMTPSocket::bind (uint16_t port) {
  assert (this->type == ROUTER);
  assert (this->state == INIT);

  this->server = new TCPServer(port);
  assert (this->server);

  this->server->begin();
  this->state = LISTENING;
  Serial.printf ("Listening on port %u...\n", port);
}

void ZMTPSocket::update () {
  Particle.process();

  if (this->state == CONNECTING) {
    this->connect(this->peer_address, this->peer_port);
  }

  if (!this->socket.connected()) {
    if (this->state == LISTENING) {
      this->socket = this->server->available();
      Serial.println ("Client acquired.");

      this->sendGreeting ();
      this->print ();
    }
    return;
  }

  while (this->socket.available ()) {
    uint8_t buffer[256];
    int bytes_read = this->socket.read (buffer, 256);
    Serial.printf ("Bytes read: %d\n", bytes_read);

    Serial.print ("Bytes: ");
    zmtp_debug_dump (buffer, bytes_read);

    switch (this->state) {
      case SIG_WAIT:
      case SIG_ACK:
      case VER_ACK:
      case GRT_ACK:
        this->state = parse_greeting (this->state, buffer, bytes_read);

        this->print ();

        if (this->state == GRT_ACK) {
          Serial.println ("Received ZMTP greeting.");
          this->sendHandshake ();
          this->state = READY_WAIT;
        }
        break;

      case READY_WAIT:
        this->state = parse_handshake (buffer, bytes_read);
        if (this->state == READY) {
          Serial.println ("Received ZMTP handshake.");
        }
        break;

      case READY:
        Serial.println ("Message frame.");
        this->frame_queue.append (zmtp_frame_new (buffer + 2, buffer[1], (zmtp_frame_flags_t) buffer[0]));
        break;

      case INIT:
      case _ERROR:
      default:
        break;
    }
  }
}

void ZMTPSocket::send (zmtp_frame_t *frame) {
  assert (this->state == READY);
  assert (frame);

  zmtp_debug_dump (zmtp_frame_bytes (frame), zmtp_frame_size (frame));
  int written = this->socket.write (zmtp_frame_bytes (frame), zmtp_frame_size (frame));
  Serial.printf ("Bytes written: %i\n", written);
  this->socket.flush ();
}

zmtp_frame_t * ZMTPSocket::recv () {
  assert (this->state == READY);

  this->update();

  if (!this->frame_queue.size()) {
    return NULL;
  }

  return this->frame_queue.takeFirst();
}

void ZMTPSocket::print () {
  zmtp_debug_dump (this->type);
  zmtp_debug_dump (this->state);
}

void ZMTPSocket::sendGreeting () {
  uint8_t greeting[64];
  memset (greeting, 0, 64);

  // signature
  greeting[0] = 0xFF;
  greeting[9] = 0x7F;

  // version
  greeting[10] = 0x03;
  greeting[11] = 0x00;

  // mechanism
  memcpy (greeting + 12, "NULL", 4);

  // as-server
  greeting[32] = this->type == DEALER ? 0x00 : 0x01;

  zmtp_debug_dump (greeting, 64);

  int written = this->socket.write (greeting, 64);
  Serial.printf ("Bytes written: %i\n", written);
  this->socket.flush ();

  this->state = SIG_WAIT;
}

void ZMTPSocket::sendHandshake () {
  size_t handshake_size = 43 + this->identity_size;
  uint8_t handshake[handshake_size];
  memset (handshake, 0, handshake_size);

  // command-size
  handshake[0] = 0x04;
  handshake[1] = handshake_size - 2;

  // ready
  handshake[2] = 0x05;
  memcpy (handshake + 3, "READY", 5);

  // metadata
  handshake[8] = 11;
  memcpy (handshake + 9, "Socket-Type", 11);
  handshake[23] = 6;
  if (this->type == DEALER) {
    memcpy (handshake + 24, "DEALER", 6);
  } else {
    memcpy (handshake + 24, "ROUTER", 6);
  }

  handshake[30] = 8;
  memcpy (handshake + 31, "Identity", 8);
  handshake[42] = this->identity_size;
  memcpy (handshake + 43, this->identity, this->identity_size);

  zmtp_debug_dump (handshake, handshake_size);

  int written = this->socket.write (handshake, handshake_size);
  Serial.printf ("Bytes written: %i\n", written);
  this->socket.flush();
}
