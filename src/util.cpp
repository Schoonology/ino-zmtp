#include "util.h"

const Logger ZMTPLog("zmtp");

void __zmtp_debug_dump(const char *label, const uint8_t *buffer, uint8_t len) {
  assert(label);
  assert(buffer);

  if (!ZMTPLog.isTraceEnabled()) {
    return;
  }

  size_t labelSize = strlen(label);
  char logMessage[labelSize + 4 + (len * 2)];
  sprintf(logMessage, "%s = ", label);
  for (uint8_t i = 0; i < len; i++) {
    sprintf(logMessage + labelSize + 3 + (i * 2), "%02X", buffer[i]);
  }
  ZMTPLog.trace("%s", logMessage);
}
