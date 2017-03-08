/**
 * Include necessary system headers here. If system APIs need to be
 * normalized across platforms, that normalization should occur here.
 */

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#elif defined(SPARK)
  #include "application.h"
#else
  #error Only ARDUINO or SPARK supported.
#endif
