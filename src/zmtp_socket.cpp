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
}

ZMTPSocket::~ZMTPSocket () {
  delete this->identity;
}

void ZMTPSocket::setIdentity (uint8_t *data, size_t size) {
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
  assert (this->state == INIT);
  assert (addr);

  bool success = this->socket.connect (IPAddress (addr), port);

  if (success) {
    Serial.println ("Connected.");

    assert (this->type == DEALER);
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
    greeting[32] = 0x00;

    zmtp_debug_dump (greeting, 64);

    this->socket.write (greeting, 64);
    this->socket.flush ();

    this->state = SIG_WAIT;
  } else {
    Serial.println ("Failed to connect.");

    this->state = _ERROR;
  }

  this->print ();

  return success;
}

void ZMTPSocket::update () {
  if (this->socket.available ()) {
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

  this->socket.write (zmtp_frame_bytes (frame), zmtp_frame_size (frame));
  this->socket.flush ();
}

void ZMTPSocket::print () {
  zmtp_debug_dump (this->type);
  zmtp_debug_dump (this->state);
}

void ZMTPSocket::sendHandshake () {
  uint8_t handshake[60];

  // command-size
  handshake[0] = 0x04;
  handshake[1] = 58;

  // ready
  handshake[2] = 0x05;
  memcpy (handshake + 3, "READY", 5);

  // metadata
  handshake[8] = 11;
  memcpy (handshake + 9, "Socket-Type", 11);
  memset (handshake + 20, 0, 4);
  handshake[23] = 6;
  memcpy (handshake + 24, "DEALER", 6);

  handshake[30] = 8;
  memcpy (handshake + 31, "Identity", 8);
  memset (handshake + 39, 0, 4);
  handshake[42] = this->identity_size + 1;
  handshake[43] = 1;
  memcpy (handshake + 44, this->identity, this->identity_size);

  zmtp_debug_dump (handshake, 60);

  this->socket.write (handshake, 60);
  this->socket.flush ();
}
