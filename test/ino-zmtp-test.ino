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
  Serial.println("");
  Serial.println("DEALER tests:");
  Serial.println("=============");

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
  Serial.println("");
  Serial.println("ROUTER tests:");
  Serial.println("=============");

  ZMTPRouter router(1235);

  //
  // Receive first message (identity first)
  //
  ZMTPFrame *firstIdentity = waitUntilRecv(router);
  if (!firstIdentity) {
    Serial.println("Router did not receive identity frame in time.");
    end();
  }

  Serial.printf("Received bytes: %u\n", firstIdentity->size());
  if (firstIdentity->size() != 6 ||
      memcmp(firstIdentity->data(), "Helper", firstIdentity->size()) != 0) {
    Serial.println("Invalid identity received.");
    end();
  }

  ZMTPFrame *question = waitUntilRecv(router);
  if (!question) {
    Serial.println("Router did not receive message frame in time.");
    end();
  }

  Serial.printf("Received bytes: %u\n", question->size());
  if (question->size() != 7 ||
      memcmp(question->data(), "Answer?", question->size()) != 0) {
    Serial.println("Invalid bytes received.");
    end();
  }

  //
  // Receive second message(identity first)
  //
  ZMTPFrame *secondIdentity = waitUntilRecv(router);
  if (!secondIdentity) {
    Serial.println("Router did not receive identity frame in time.");
    router.print();
    end();
  }

  Serial.printf("Received bytes: %u\n", secondIdentity->size());
  if (secondIdentity->size() != 7 ||
      memcmp(secondIdentity->data(), "Another", secondIdentity->size()) != 0) {
    Serial.println("Invalid identity received.");
    end();
  }

  ZMTPFrame *ping = waitUntilRecv(router);
  if (!ping) {
    Serial.println("Router did not receive message frame in time.");
    end();
  }

  Serial.printf("Received bytes: %u\n", ping->size());
  if (ping->size() != 4 || memcmp(ping->data(), "ping", ping->size()) != 0) {
    Serial.println("Invalid bytes received.");
    end();
  }

  //
  // Send responses
  //
  router.send(firstIdentity);

  ZMTPFrame firstResponse("42", ZMTP_FRAME_NONE);
  router.send(&firstResponse);

  router.send(secondIdentity);

  ZMTPFrame secondResponse("pong", ZMTP_FRAME_NONE);
  router.send(&secondResponse);
}

void setup() {
  enableWiFi();

  uint8_t address[4] = {192, 168, 29, 198};
  // uint8_t address[4] = {172, 20, 10, 3};
  runDealerTest(address);
  runRouterTest();

  end();
}

void end() {
  Serial.println("Tearing down.");
  Serial.flush();
  System.dfu();
}
