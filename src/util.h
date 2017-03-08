#ifndef __UTIL_H_INCLUDED__
#define __UTIL_H_INCLUDED__

#include "arch.h"

#undef assert
#undef __assert
#define assert(e) \
  ((void) ((e) ? ((void) 0) : __assert (#e, __FILE__, __LINE__)))
#define __assert(e, file, line) \
  ((void) Serial.printf ("%s:%u: failed assertion '%s'\n", file, line, e), \
    abort ())

#define VAR_ARG_MACRO(_1,_2,NAME,...) NAME
#define debug_dump(...) VAR_ARG_MACRO(__VA_ARGS__, debug_dump2, debug_dump1)(__VA_ARGS__)

#define debug_dump1(e) \
  ((void) (Serial.printf("%s = ", #e) && Serial.println(e) ))

#define debug_dump2(e, len) \
  ((void) __debug_dump (#e, e, len))
void __debug_dump (const char *label, const uint8_t *buffer, uint8_t len);

void zero_bytes (uint8_t *buffer, uint8_t offset, uint8_t length);

uint8_t *uuid_new ();

bool uuid_equal (uint8_t *first, uint8_t *second);

#endif
