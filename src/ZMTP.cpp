/* zMTP library by Michael Schoonmaker
 */

#include "zMTP.h"

/**
 * Constructor.
 */
ZMTP::ZMTP()
{
  // be sure not to call anything that requires hardware be initialized here, put those in begin()
}

/**
 * Example method.
 */
void ZMTP::begin()
{
    // initialize hardware
    Serial.println("called begin");
}

/**
 * Example method.
 */
void ZMTP::process()
{
    // do something useful
    Serial.println("called process");
    doit();
}

/**
* Example private method
*/
void ZMTP::doit()
{
    Serial.println("called doit");
}
