#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <DHT.h>

const byte PIN_DHT = 4;
const byte PIN_RELAY = 5;
const char *SSID_NAME = "SmartHome";
float SETPOINT = 33.0;
float HYSTER = 3;
bool RELAY_STATE = false;

ESP8266WebServer HTTP(80);
DHT dht(PIN_DHT, DHT11);

void setup() {
  pinMode(PIN_RELAY, OUTPUT);
  Serial.begin(9600);

  WiFi.softAP(SSID_NAME);

  SPIFFS.begin();
  HTTP.begin();
  dht.begin();

  Serial.println("\nSmartHome");
  Serial.print("IP-адрес устройства: ");
  Serial.print(WiFi.softAPIP());
  Serial.println("\n");

  HTTP.on("/temperature", []() {
    HTTP.send(200, "text/plain", temperature());
  });

  HTTP.on("/relay_status", []() {
    HTTP.send(200, "text/plain", relay_status());
  });

  HTTP.onNotFound([]() {
    if (!handleFileRead(HTTP.uri()))
      HTTP.send(404, "text/plain", "Not Found");
  });
}

void loop() {
  HTTP.handleClient();

  if (dht.readTemperature() > (SETPOINT + HYSTER)) RELAY_STATE = true;
  else if (dht.readTemperature() < (SETPOINT - HYSTER)) RELAY_STATE = false;  
  digitalWrite(PIN_RELAY, RELAY_STATE);
}

String temperature() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    return String(t) + "|" + String(h);
  } else {
    return String("NaN");
  }
}

String relay_status() {
  return String(digitalRead(PIN_RELAY));
}

bool handleFileRead(String path) {
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    size_t sent = HTTP.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".svg")) return "image/svg+xml";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}
