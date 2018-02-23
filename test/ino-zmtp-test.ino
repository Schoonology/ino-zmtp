#include "../src/ZMTP.h"

SYSTEM_MODE(MANUAL);

void waitUntilReady(ZMTPSocket &socket) {
  System.waitCondition(
      [&]() {
        // Removing this will crash the application, as socket.ready()
        // returns true, but socket.recv() won't agree.
        delay(100);
        socket.update();
        return socket.ready();
      },
      5000);
}

zmtp_frame_t *waitUntilRecv(ZMTPSocket &socket) {
  waitUntilReady(socket);

  zmtp_frame_t *frame = NULL;
  unsigned long startTime = millis();
  do {
    socket.update();
    frame = socket.recv();
  } while (startTime - millis() < 5000 && !frame);

  return frame;
}

void enableWiFi() {
  WiFi.on();
  WiFi.connect();
  waitUntil(WiFi.ready);

  // Particle.process is required to acquire an IP address.
  Particle.process();

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void runDealerTest(uint8_t *address) {
  ZMTPSocket dealer(DEALER);

  Serial.printf("Connecting to: %i.%i.%i.%i\n", address[0], address[1],
                address[2], address[3]);
  dealer.connect(address, 1234);

  uint8_t identity[6] = {'P', 'h', 'o', 't', 'o', 'n'};
  dealer.setIdentity(identity, 6);

  waitUntilReady(dealer);

  {
    uint8_t data[5] = {'H', 'e', 'l', 'l', 'o'};
    zmtp_frame_t *frame = zmtp_frame_new(data, 5);
    dealer.send(frame);
    zmtp_frame_destroy(&frame);
  }

  {
    IPAddress localIP = WiFi.localIP();
    uint8_t localIPBytes[4] = {localIP[0], localIP[1], localIP[2], localIP[3]};
    zmtp_frame_t *frame = zmtp_frame_new(localIPBytes, 4);
    dealer.send(frame);
    zmtp_frame_destroy(&frame);
  }
}

void runRouterTest() {
  ZMTPSocket router(ROUTER);

  router.bind(1235);

  zmtp_frame_t *question = waitUntilRecv(router);

  // TODO(schoon) - Receive identity frame first.
  // HACK(schoon) - The zmtp_frame API doesn't tell us about framing, so
  // we have to manually account for it here.
  uint8_t question_size = zmtp_frame_size(question) - 2;
  const char *question_bytes = (const char *)(zmtp_frame_bytes(question) + 2);
  Serial.printf("Received bytes: %u\n", question_size);

  if (strcmp(question_bytes, "Answer?") != 0) {
    Serial.println("Invalid bytes received.");
    end();
  }

  {
    // TODO(schoon) - "Send" identity frame first.
    uint8_t data[2] = {'4', '2'};
    zmtp_frame_t *frame = zmtp_frame_new(data, 2);
    router.send(frame);
    zmtp_frame_destroy(&frame);
  }
}

void setup() {
  enableWiFi();

  // uint8_t address[4] = { 192, 168, 29, 198 };
  uint8_t address[4] = {172, 20, 10, 3};
  runDealerTest(address);
  runRouterTest();

  // Allow time for network to catch up before we hard disconnect.
  delay(1000);

  end();
}

void end() {
  Serial.println("Tearing down.");
  Serial.flush();
  System.dfu();
}
