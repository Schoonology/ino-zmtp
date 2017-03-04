#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#elif defined(SPARK)
  #include "application.h"
#else
  #error Only ARDUINO or SPARK supported.
#endif
