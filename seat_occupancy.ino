#include <WiFi.h>
#include <PubSubClient.h>
#include "time.h"

// ─── Pin Definitions ───────────────────────────────────────────────────────────
#define TRIG_PIN 25
#define ECHO_PIN 13

// ─── WiFi Credentials ──────────────────────────────────────────────────────────
const char* ssid     = "#ssid";
const char* password = "#pwd";

// ─── MQTT Broker ───────────────────────────────────────────────────────────────
const char* mqttServer = "192.168.1.100";  // Replace with your broker IP
const int   mqttPort   = 1883;
const char* mqttTopic  = "iotb/team1/chair1";  // Change team/chair number as needed

// ─── NTP / Time ────────────────────────────────────────────────────────────────
const char* ntpServer        = "in.pool.ntp.org";
const long  gmtOffset_sec    = 19800;  // IST = UTC+5:30
const int   daylightOffset_sec = 0;

// ─── Occupancy Threshold ───────────────────────────────────────────────────────
const int OCCUPANCY_THRESHOLD_CM = 10;  // Distance below which seat is occupied

// ─── Clients ───────────────────────────────────────────────────────────────────
WiFiClient   espClient;
PubSubClient mqttClient(espClient);

// ──────────────────────────────────────────────────────────────────────────────
// Get distance from ultrasonic sensor in cm
// Returns 999 if no echo detected (timeout)
// ──────────────────────────────────────────────────────────────────────────────
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);  // 30ms timeout
  if (duration == 0) return 999;                    // No echo = nothing detected

  return duration * 0.034 / 2;  // Convert to cm
}

// ──────────────────────────────────────────────────────────────────────────────
// Get current IST time as formatted string
// ──────────────────────────────────────────────────────────────────────────────
String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "TimeError";
  }
  char timeString[30];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeString);
}

// ──────────────────────────────────────────────────────────────────────────────
// Reconnect to MQTT broker if disconnected
// ──────────────────────────────────────────────────────────────────────────────
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT broker...");
    String clientId = "ESP32_Chair1_" + String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println(" connected!");
    } else {
      Serial.print(" failed. State=");
      Serial.print(mqttClient.state());
      Serial.println(" | Retrying in 3 seconds...");
      delay(3000);
    }
  }
}

// ──────────────────────────────────────────────────────────────────────────────
// Setup
// ──────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());

  // Sync time from NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time synced from NTP.");

  // Setup MQTT
  mqttClient.setServer(mqttServer, mqttPort);
}

// ──────────────────────────────────────────────────────────────────────────────
// Main Loop
// ──────────────────────────────────────────────────────────────────────────────
void loop() {
  // Ensure MQTT is connected
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  // Read sensor
  long distance    = getDistance();
  String timestamp = getTimeString();
  bool occupied    = (distance <= OCCUPANCY_THRESHOLD_CM);

  // Build JSON payload
  String payload = "{";
  payload += "\"Chair\":1,";
  payload += "\"Type\":\"Ultrasonic\",";
  payload += "\"Distance_cm\":" + String(distance) + ",";
  payload += "\"Status\":\"" + String(occupied ? "True" : "False") + "\",";
  payload += "\"Time\":\"" + timestamp + "\"";
  payload += "}";

  // Publish to MQTT
  bool published = mqttClient.publish(mqttTopic, payload.c_str());

  // Debug output
  Serial.println(payload);
  Serial.println(published ? "Published OK" : "Publish FAILED");

  delay(1000);
}
