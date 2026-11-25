// ESP32 + DHT22 → Google Sheets logger
// Reads temperature and humidity, sends them to a Google Apps Script web app every 4 hours.
// WiFi credentials and the web app URL live in secrets.h (not committed; copy secrets.example.h).

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include "secrets.h"

#define DHTPIN 4        // GPIO4 — do not use input-only pins 34/35/36/39
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const unsigned long SEND_INTERVAL = 4UL * 60UL * 60UL * 1000UL;  // 4 hours
const unsigned long RETRY_DELAY = 10000UL;                       // check / retry every 10 s
unsigned long lastSend = 0;
bool firstSend = true;

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting WiFi");
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 40) {  // ~20 s
    delay(500);
    Serial.print(".");
    tries++;
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? " OK" : " FAIL");
}

void sendData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("DHT22 read failed, retrying");
    return;
  }

  connectWiFi();
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();  // skip certificate check (fine for this logger)
  HTTPClient http;
  String url = String(SCRIPT_URL) + "?temperature=" + String(t, 1) + "&humidity=" + String(h, 1);
  http.begin(client, url);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);  // Apps Script answers with a redirect
  int code = http.GET();
  Serial.printf("Temp %.1f Hum %.1f -> HTTP %d %s\n", t, h, code, http.getString().c_str());
  http.end();

  if (code == 200) {
    lastSend = millis();
    firstSend = false;
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  delay(2000);  // let the sensor settle
  connectWiFi();
}

void loop() {
  if (firstSend || millis() - lastSend >= SEND_INTERVAL) {
    sendData();
  }
  delay(RETRY_DELAY);
}
