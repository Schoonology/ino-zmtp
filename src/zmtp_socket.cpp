#include "zmtp_socket.h"
#include "util.h"

typedef enum {
  INIT = 0,   // New connection
  SIG_WAIT,   // ZMTP signature sent
  SIG_ACK,    // ZMTP signature acknowledged
  VER_ACK,    // ZMTP version acknowledged
  GRT_ACK,    // ZMTP greeting acknowledged
  READY_WAIT, // ZMTP READY command sent
  READY,      // ZMTP READY command acknowledged
  _ERROR,     // Connection or ZMTP handshake failed
} zmtp_socket_state_t;

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

struct _zmtp_socket_t {
  zmtp_socket_type_t type;
  zmtp_socket_state_t state;
  zmtp_uuid_t *uuid;
  TCPClient *socket;
};

zmtp_socket_t *zmtp_socket_new (zmtp_socket_type_t type) {
  zmtp_socket_t *self = (zmtp_socket_t *) malloc (sizeof (zmtp_socket_t));
  assert (self);

  self->type = type;
  self->state = INIT;
  self->uuid = zmtp_uuid_new ();

  self->socket = new TCPClient();
  assert (self->socket);

  return self;
}

void zmtp_socket_destroy (zmtp_socket_t **self_p) {
  assert (self_p);

  if (*self_p) {
    zmtp_socket_t *self = *self_p;

    zmtp_uuid_destroy (&self->uuid);
    delete self->socket;

    free (self);
    *self_p = NULL;
  }
}

void zmtp_socket_uuid (zmtp_socket_t *self, zmtp_uuid_t *uuid) {
  assert (self);
  assert (uuid);

  memcpy (zmtp_uuid_bytes (self->uuid), zmtp_uuid_bytes (uuid), ZMTP_UUID_LENGTH);
}

bool zmtp_socket_ready (zmtp_socket_t *self) {
  return self->state == READY;
}

bool zmtp_socket_connect (zmtp_socket_t *self, uint8_t *addr, uint16_t port) {
  assert (self);
  assert (self->state == INIT);
  assert (addr);

  bool success = self->socket->connect (IPAddress (addr), port);

  if (success) {
    Serial.println ("Connected.");

    assert (self->type == DEALER);
    uint8_t greeting[64];
    zero_bytes (greeting, 0, 64);

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

    debug_dump (greeting, 64);

    self->socket->write (greeting, 64);
    self->socket->flush ();

    self->state = SIG_WAIT;
  } else {
    Serial.println ("Failed to connect.");

    self->state = _ERROR;
  }

  zmtp_socket_dump (self);

  return success;
}

void zmtp_send_handshake (zmtp_socket_t *self) {
  assert (self);

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
  zero_bytes (handshake, 20, 4);
  handshake[23] = 6;
  memcpy (handshake + 24, "DEALER", 6);

  handshake[30] = 8;
  memcpy (handshake + 31, "Identity", 8);
  zero_bytes (handshake, 39, 4);
  handshake[42] = 17;
  handshake[43] = 1;
  memcpy (handshake + 44, zmtp_uuid_bytes (self->uuid), 16);

  debug_dump (handshake, 60);

  self->socket->write (handshake, 60);
  self->socket->flush ();
}

void zmtp_socket_update (zmtp_socket_t *self) {
  assert (self);

  if (self->socket->available ()) {
    uint8_t buffer[256];
    int bytes_read = self->socket->read (buffer, 256);
    Serial.printf ("Bytes read: %d\n", bytes_read);

    Serial.print ("Bytes: ");
    debug_dump (buffer, bytes_read);

    switch (self->state) {
      case SIG_WAIT:
      case SIG_ACK:
      case VER_ACK:
      case GRT_ACK:
        self->state = parse_greeting (self->state, buffer, bytes_read);

        zmtp_socket_dump (self);

        if (self->state == GRT_ACK) {
          Serial.println ("Received ZMTP greeting.");
          zmtp_send_handshake (self);
          self->state = READY_WAIT;
        }
        break;

      case READY_WAIT:
        self->state = parse_handshake (buffer, bytes_read);
        if (self->state == READY) {
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

void zmtp_socket_send (zmtp_socket_t *self, zmtp_frame_t *frame) {
  assert (self);
  assert (self->state == READY);
  assert (frame);

  self->socket->write (zmtp_frame_bytes (frame), zmtp_frame_size (frame));
  self->socket->flush ();
}

void zmtp_socket_dump (zmtp_socket_t *self) {
  assert (self);

  debug_dump (self->type);
  debug_dump (self->state);
}
