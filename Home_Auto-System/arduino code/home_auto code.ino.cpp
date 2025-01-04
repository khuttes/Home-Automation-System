#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <AceButton.h>
using namespace ace_button;

// Replace these with your actual values
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"
#define API_KEY "YourSinricAPIKey"
#define HEARTBEAT_INTERVAL 300000 // 5 minutes

// Pin definitions
#define BUTTON1_PIN 14
#define BUTTON2_PIN 12
#define BUTTON3_PIN 13
#define BUTTON4_PIN 3
#define RELAY1_PIN 5
#define RELAY2_PIN 4
#define RELAY3_PIN 0
#define RELAY4_PIN 2

// Device IDs
const String DEVICE_ID_1 = "DeviceID1";
const String DEVICE_ID_2 = "DeviceID2";
const String DEVICE_ID_3 = "DeviceID3";
const String DEVICE_ID_4 = "DeviceID4";

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
uint64_t heartbeatTimestamp = 0;
bool isConnected = false;

ButtonConfig button1Config, button2Config, button3Config, button4Config;
AceButton button1(&button1Config);
AceButton button2(&button2Config);
AceButton button3(&button3Config);
AceButton button4(&button4Config);

void handleButton1(AceButton*, uint8_t, uint8_t);
void handleButton2(AceButton*, uint8_t, uint8_t);
void handleButton3(AceButton*, uint8_t, uint8_t);
void handleButton4(AceButton*, uint8_t, uint8_t);
void turnOn(String deviceId);
void turnOff(String deviceId);
void sendPowerStateToServer(String deviceId, String value);

void setup() {
  Serial.begin(115200);

  // Initialize pins
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(BUTTON4_PIN, INPUT_PULLUP);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);

  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(RELAY3_PIN, HIGH);
  digitalWrite(RELAY4_PIN, HIGH);

  // Set up button handlers
  button1Config.setEventHandler(handleButton1);
  button2Config.setEventHandler(handleButton2);
  button3Config.setEventHandler(handleButton3);
  button4Config.setEventHandler(handleButton4);

  button1.init(BUTTON1_PIN);
  button2.init(BUTTON2_PIN);
  button3.init(BUTTON3_PIN);
  button4.init(BUTTON4_PIN);

  // Connect to Wi-Fi
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wi-Fi...");

  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Connect to Sinric WebSocket
  webSocket.begin("iot.sinric.com", 80, "/");
  webSocket.onEvent([](WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
      case WStype_DISCONNECTED:
        isConnected = false;
        Serial.println("Disconnected from Sinric.");
        break;
      case WStype_CONNECTED:
        isConnected = true;
        Serial.println("Connected to Sinric.");
        break;
      case WStype_TEXT: {
        DynamicJsonDocument json(1024);
        deserializeJson(json, payload);

        String deviceId = json["deviceId"];
        String action = json["action"];

        if (action == "action.devices.commands.OnOff") {
          bool state = json["value"]["on"];
          if (state) {
            turnOn(deviceId);
          } else {
            turnOff(deviceId);
          }
        }
      } break;
      default:
        break;
    }
  });

  webSocket.setAuthorization("apikey", API_KEY);
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();
  uint64_t now = millis();

  if (isConnected && now - heartbeatTimestamp > HEARTBEAT_INTERVAL) {
    heartbeatTimestamp = now;
    webSocket.sendTXT("H");
  }

  button1.check();
  button2.check();
  button3.check();
  button4.check();
}

void turnOn(String deviceId) {
  if (deviceId == DEVICE_ID_1) digitalWrite(RELAY1_PIN, LOW);
  else if (deviceId == DEVICE_ID_2) digitalWrite(RELAY2_PIN, LOW);
  else if (deviceId == DEVICE_ID_3) digitalWrite(RELAY3_PIN, LOW);
  else if (deviceId == DEVICE_ID_4) digitalWrite(RELAY4_PIN, LOW);

  Serial.println("Turned on: " + deviceId);
}

void turnOff(String deviceId) {
  if (deviceId == DEVICE_ID_1) digitalWrite(RELAY1_PIN, HIGH);
  else if (deviceId == DEVICE_ID_2) digitalWrite(RELAY2_PIN, HIGH);
  else if (deviceId == DEVICE_ID_3) digitalWrite(RELAY3_PIN, HIGH);
  else if (deviceId == DEVICE_ID_4) digitalWrite(RELAY4_PIN, HIGH);

  Serial.println("Turned off: " + deviceId);
}

void sendPowerStateToServer(String deviceId, String value) {
  DynamicJsonDocument json(1024);
  json["deviceId"] = deviceId;
  json["action"] = "setPowerState";
  json["value"] = value;

  String jsonString;
  serializeJson(json, jsonString);
  webSocket.sendTXT(jsonString);
}

void handleButton1(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if (eventType == AceButton::kEventPressed) {
    sendPowerStateToServer(DEVICE_ID_1, "ON");
    turnOn(DEVICE_ID_1);
  } else if (eventType == AceButton::kEventReleased) {
    sendPowerStateToServer(DEVICE_ID_1, "OFF");
    turnOff(DEVICE_ID_1);
  }
}

void handleButton2(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if (eventType == AceButton::kEventPressed) {
    sendPowerStateToServer(DEVICE_ID_2, "ON");
    turnOn(DEVICE_ID_2);
  } else if (eventType == AceButton::kEventReleased) {
    sendPowerStateToServer(DEVICE_ID_2, "OFF");
    turnOff(DEVICE_ID_2);
  }
}

void handleButton3(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if (eventType == AceButton::kEventPressed) {
    sendPowerStateToServer(DEVICE_ID_3, "ON");
    turnOn(DEVICE_ID_3);
  } else if (eventType == AceButton::kEventReleased) {
    sendPowerStateToServer(DEVICE_ID_3, "OFF");
    turnOff(DEVICE_ID_3);
  }
}

void handleButton4(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if (eventType == AceButton::kEventPressed) {
    sendPowerStateToServer(DEVICE_ID_4, "ON");
    turnOn(DEVICE_ID_4);
  } else if (eventType == AceButton::kEventReleased) {
    sendPowerStateToServer(DEVICE_ID_4, "OFF");
    turnOff(DEVICE_ID_4);
  }
}
