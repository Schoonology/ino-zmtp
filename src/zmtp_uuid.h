/**
 * A 16-byte UUID valid as a ZMTP identity.
 */

#ifndef __ZMTP_UUID_H_INCLUDED__
#define __ZMTP_UUID_H_INCLUDED__

#include "arch.h"

#define ZMTP_UUID_LENGTH 16

// Opaque struct.
typedef struct _zmtp_uuid_t zmtp_uuid_t;

// Create a new, random UUID.
zmtp_uuid_t *zmtp_uuid_new ();

// Destroy a UUID.
void zmtp_uuid_destroy (zmtp_uuid_t **self_p);

// Compare two UUIDs, returning true if the two are equal,
// false otherwise.
bool zmtp_uuid_equal (zmtp_uuid_t *first, zmtp_uuid_t *second);

// Return a pointer to the underlying 16 bytes.
uint8_t *zmtp_uuid_bytes (zmtp_uuid_t *self);

// Dump internal state.
void zmtp_uuid_dump (zmtp_uuid_t *self);

#endif
