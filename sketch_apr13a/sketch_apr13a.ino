#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "WiFi_SSID";
const char* password = "WiFi_PASSWORD";
const char* serverName = "http://127.0.0.1:9200/upgrade_pid"; 

void setup() {
  Serial.begin(115200);
  delay(100);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Payload: " + payload);

      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        float kp = doc["P"];
        float ki = doc["I"];
        float kd = doc["D"];

        Serial.println("Received PID values:");
        Serial.print("P: "); Serial.println(kp);
        Serial.print("I: "); Serial.println(ki);
        Serial.print("D: "); Serial.println(kd);
      } else {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      }

    } else {
      Serial.print("HTTP error: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

void loop() {

}