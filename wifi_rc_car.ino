#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <Servo.h>

#include "pages.h"

#define PIN_SERVO D5
#define PIN_MOTOR_D D1
#define PIN_MOTOR_E D2

Servo servo;
ESP8266WebServer server(80);
WebSocketsServer webSocket(81);
StaticJsonDocument<50> doc;

void sendDirection (int speed) {
/*
  if (speed == 0) {
    digitalWrite(PIN_MOTOR_D, LOW);
    digitalWrite(PIN_MOTOR_E, LOW);
  } else if (speed > 0) {
    digitalWrite(PIN_MOTOR_D, HIGH);
    digitalWrite(PIN_MOTOR_E, LOW);
  } else {
    digitalWrite(PIN_MOTOR_D, LOW);
    digitalWrite(PIN_MOTOR_E, HIGH);
  }
*/
}

void sendAngle (uint angle) {
  servo.write(map(angle, 0, 180, 0, 179));
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      
        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[%u] get Text: %s\n", num, payload);
  
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
          Serial.println(error.f_str());
          return;
        }

        sendDirection(doc["speed"]);
        sendAngle(doc["angle"]);
      }
      break;
    case WStype_ERROR:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      break;
  }
}

void handleRoot() {
  server.send(200, "text/html", REMOTE_CONTROL);
}

void setup() {
  servo.attach(PIN_SERVO);
  pinMode(PIN_MOTOR_D, OUTPUT);
  pinMode(PIN_MOTOR_E, OUTPUT);
  digitalWrite(PIN_MOTOR_D, HIGH);
  digitalWrite(PIN_MOTOR_E, HIGH);

  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.softAP(APSSID, APPSWD);

  server.on("/", handleRoot);
  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  MDNS.begin("ESP8266");
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
}

void loop() {
  server.handleClient();
  webSocket.loop();
}
