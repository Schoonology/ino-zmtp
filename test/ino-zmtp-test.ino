#include "../src/ZMTP.h"

SYSTEM_MODE(MANUAL);

void setup () {
  WiFi.on();
  WiFi.connect();
  waitUntil(WiFi.ready);
  Particle.process();

  ZMTPSocket socket (DEALER);

  Serial.print ("SSID: ");
  Serial.println (WiFi.SSID());
  Serial.print ("IP Address: ");
  Serial.println (WiFi.localIP());

  // uint8_t address[4] = { 192, 168, 29, 198 };
  uint8_t address[4] = { 172, 20, 10, 3 };
  Serial.printf ("Connecting to: %i.%i.%i.%i\n", address[0], address[1], address[2], address[3]);
  socket.connect (address, 1234);

  uint8_t identity[6] = { 'P', 'h', 'o', 't', 'o', 'n' };
  socket.setIdentity (identity, 6);

  int tries = 0;
  while (!socket.ready ()) {
    Serial.println ("Not ready yet.");
    if (++tries > 5) {
      end ();
    }
    delay (1000);
    socket.update();
  }

  {
    uint8_t data[5] = { 'H', 'e', 'l', 'l', 'o' };
    zmtp_frame_t *frame = zmtp_frame_new (data, 5);
    socket.send (frame);
    zmtp_frame_destroy (&frame);
  }

  delay (1000);

  end();
}

void end() {
  Serial.println ("Tearing down.");
  Serial.flush();
  System.dfu();
}
