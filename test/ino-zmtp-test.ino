#include "../src/ZMTP.h"

zmtp_socket_t *socket;

void setup () {
  Serial.print ("IP Address: ");
  Serial.println (WiFi.localIP());

  socket = zmtp_socket_new (DEALER);
  uint8_t address[4] = { 192, 168, 29, 198 };
  zmtp_socket_connect (socket, address, 1234);

  uint8_t uuid[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
  zmtp_socket_uuid (socket, (zmtp_uuid_t *) uuid);

  int tries = 0;
  while (!zmtp_socket_ready (socket)) {
    Serial.println ("Not ready yet.");
    if (++tries > 100) {
      end ();
    }
    delay (1000);
    zmtp_socket_update (socket);
  }

  {
    uint8_t data[5] = { 'H', 'e', 'l', 'l', 'o' };
    zmtp_frame_t *frame = zmtp_frame_new (data, 5);
    zmtp_socket_send (socket, frame);
    zmtp_frame_destroy (&frame);
  }

  delay (1000);

  {
    uint8_t data[5] = { 'W', 'o', 'r', 'l', 'd' };
    zmtp_frame_t *frame = zmtp_frame_new (data, 5);
    zmtp_socket_send (socket, frame);
    zmtp_frame_destroy (&frame);
  }

  delay (1000);

  end();
}

void end() {
  Serial.println ("Tearing down.");
  zmtp_socket_destroy (&socket);

  Serial.flush();
  System.dfu();
}
