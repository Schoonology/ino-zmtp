#include "../src/ZMTP.h"

SYSTEM_MODE(MANUAL);

bool waitUntilReady(ZMTPSocket &socket) {
  return System.waitCondition(
      [&]() {
        // Removing this will crash the application, as socket.ready()
        // returns true, but socket.recv() won't agree.
        delay(100);
        socket.update();
        return socket.ready();
      },
      5000);
}

ZMTPFrame *waitUntilRecv(ZMTPSocket &socket) {
  if (!waitUntilReady(socket)) {
    return NULL;
  }

  ZMTPFrame *frame = NULL;
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

  if (!waitUntilReady(dealer)) {
    Serial.println("Dealer was not ready in time.");
    end();
  }

  {
    ZMTPFrame frame("Hello", ZMTP_FRAME_NONE);
    dealer.send(&frame);
  }

  {
    IPAddress localIP = WiFi.localIP();
    uint8_t localIPBytes[4] = {localIP[0], localIP[1], localIP[2], localIP[3]};
    ZMTPFrame frame(localIPBytes, 4, ZMTP_FRAME_NONE);
    dealer.send(&frame);
  }
}

void runRouterTest() {
  ZMTPSocket router(ROUTER);

  router.bind(1235);

  ZMTPFrame *question = waitUntilRecv(router);
  if (!question) {
    Serial.println("Router did not receive message in time.");
    end();
  }

  // TODO(schoon) - Receive identity frame first.
  Serial.printf("Received bytes: %u\n", question->size());
  if (strcmp((const char *)question->data(), "Answer?") != 0) {
    Serial.println("Invalid bytes received.");
    end();
  }

  {
    // TODO(schoon) - "Send" identity frame first.
    ZMTPFrame frame("42", ZMTP_FRAME_NONE);
    router.send(&frame);
  }
}

void setup() {
  enableWiFi();

  // uint8_t address[4] = { 192, 168, 29, 198 };
  uint8_t address[4] = {172, 20, 10, 3};
  runDealerTest(address);
  runRouterTest();

  end();
}

void end() {
  Serial.println("Tearing down.");
  Serial.flush();
  System.dfu();
}
