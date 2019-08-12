#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#define DHTPIN 23
#define DHTTYPE DHT22

const char* const SSID = "Tomato24";
const char* const PASS = "TalkischeapShowmethecode";
const char* const HOSTNAME = "ESP32-BC";

float temp = 0.0;
float mois = 0.0;
HTTPClient http;
bool wifi_connected = false;
DHT dht(DHTPIN, DHTTYPE);

void onWiFiDisconnect() {
  WiFi.disconnect();
  WiFi.begin(SSID, PASS);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
}

void WiFiEvent(WiFiEvent_t event) {
  switch (event)
  {
  case SYSTEM_EVENT_STA_START:
    WiFi.setHostname(HOSTNAME);
    break;
  case SYSTEM_EVENT_STA_CONNECTED:
    WiFi.enableIpV6();
    break;
  case SYSTEM_EVENT_GOT_IP6:
    Serial.print("Got IPv6: ");
    Serial.println(WiFi.localIPv6());
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    wifi_connected = true;
    Serial.print("Got IPv4: ");
    Serial.println(WiFi.localIP());
    digitalWrite(2, 1);
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    wifi_connected = false;
    Serial.println("WiFi disconnected");
    digitalWrite(2, 0);
    onWiFiDisconnect();
  default:
    break;
  }
}

void SendData() {
  StaticJsonDocument <200> data;
  data["temp"] = temp;
  data["humidity"] = mois;
  http.begin("uphan.makerspace.nqdclub.com",80,"/postjson");
  http.addHeader("Content-Type","application/json");
  String posthttp;
  serializeJson(data,posthttp);
  Serial.println(posthttp);
  int httpCode = http.POST(posthttp);
  Serial.print("httpCode: "); Serial.println(httpCode);
  http.end();
}

void WiFiConnectedLoop() {
  temp = dht.readTemperature(false);
  delay(1000);
  mois = 100.0 - ((analogRead(35) * 100.0) / 4096.0);
  Serial.print("Temperature: "); Serial.println(temp);
  Serial.print("Moisture: "); Serial.println(mois);
  SendData();
}


void setup() {
  // Serial Interface for debugging
  Serial.begin(9600);
  Serial.println("Starting up... please wait...");

  // Onboard LED for signaling

  pinMode(2, OUTPUT);
  
  // Init DHT sensor
  dht.begin();
  
  // Debug
  Serial.println("Connecting to WiFi...");
  
  // Begin connecting to WiFi
  WiFi.disconnect(); // Disconnect to any WiFi, ESP shouldn't connected to any AP at this point
  WiFi.onEvent(WiFiEvent); // Event handler for WiFiEvent
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin(SSID, PASS);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && wifi_connected == true) {
    WiFiConnectedLoop();
  }
}