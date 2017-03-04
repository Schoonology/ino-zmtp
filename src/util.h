#ifndef __UTIL_H_INCLUDED__
#define __UTIL_H_INCLUDED__

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#elif defined(SPARK)
  #include "application.h"
#else
  #error Only ARDUINO or SPARK supported.
#endif

#undef assert
#undef __assert
#define assert(e) \
  ((void) ((e) ? ((void) 0) : __assert (#e, __FILE__, __LINE__)))
#define __assert(e, file, line) \
  ((void) Serial.printf ("%s:%u: failed assertion '%s'\n", file, line, e), \
    abort ())

#define debug_dump(e, len) \
  ((void) __debug_dump (#e, e, len))
void __debug_dump (const char *label, const uint8_t *buffer, uint8_t len);

#endif
