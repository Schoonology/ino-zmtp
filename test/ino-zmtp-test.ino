#include "../src/ZMTP.h"

SYSTEM_MODE(MANUAL);

unsigned long TIMEOUT = 30000;

bool waitUntilSend(ZMTPSocket &socket, ZMTPFrame *frame) {
  bool success = false;
  unsigned long startTime = millis();
  do {
    success = socket.send(frame);
  } while (millis() - startTime < TIMEOUT && !success);

  return success;
}

ZMTPFrame *waitUntilRecv(ZMTPSocket &socket) {
  ZMTPFrame *frame = NULL;
  unsigned long startTime = millis();
  do {
    frame = socket.recv();
  } while (millis() - startTime < TIMEOUT && !frame);

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
  Serial.printf("Connecting to: %i.%i.%i.%i\n", address[0], address[1],
                address[2], address[3]);
  ZMTPDealer dealer(address, 1234);

  uint8_t identity[6] = {'P', 'h', 'o', 't', 'o', 'n'};
  dealer.setIdentity(identity, 6);

  {
    ZMTPFrame frame("Hello", ZMTP_FRAME_NONE);

    if (!waitUntilSend(dealer, &frame)) {
      Serial.println("Dealer did not send greeting in time.");
      end();
    }
  }

  {
    IPAddress localIP = WiFi.localIP();
    uint8_t localIPBytes[4] = {localIP[0], localIP[1], localIP[2], localIP[3]};
    ZMTPFrame frame(localIPBytes, 4, ZMTP_FRAME_NONE);

    if (!waitUntilSend(dealer, &frame)) {
      Serial.println("Dealer did not send IP address in time.");
      end();
    }
  }
}

void runRouterTest() {
  ZMTPRouter router(1235);

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
