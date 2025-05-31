#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

ESP8266WebServer server(9200);

void handleUpgradePID() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Body not received");
    return;
  }

  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (!doc.containsKey("P") || !doc.containsKey("I") || !doc.containsKey("D")) {
    server.send(400, "application/json", "{\"error\":\"Missing PID parameters\"}");
    return;
  }

  float kp = doc["P"];
  float ki = doc["I"];
  float kd = doc["D"];

  Serial.println("Received PID:");
  Serial.print("P: "); Serial.println(kp);
  Serial.print("I: "); Serial.println(ki);
  Serial.print("D: "); Serial.println(kd);

  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("Sasha224", "115819qw");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("ESP IP Address: ");
  Serial.println(WiFi.localIP());  
  server.on("/upgrade_pid", HTTP_POST, handleUpgradePID);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}