#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>

// ─── USER CONFIG ───────────────────────────────────────────────────────────────
const char*    wifiName   = "#ssid";
const char*    wifiKey    = "#pwd";
const char*    mqtt_server = "test.mosquitto.org";  // Replace with local broker IP if available
const uint16_t mqtt_port  = 1883;
const char*    mqtt_topic = "iotb/team2/chair1";    // Change team/chair as needed
const uint8_t  chairId    = 1;

// Touch sensor config
// Safe touch pins on ESP32: 4, 13, 14, 27, 32, 33
const uint8_t  TOUCH_PIN  = 4;
const uint16_t threshold  = 1000;   

// Publish interval
const unsigned long PUBLISH_INTERVAL = 2000;  // 2 seconds

// NTP / IST timezone
const char* ntpServer          = "pool.ntp.org";
const long  gmtOffset_sec      = 19800;  
const int   daylightOffset_sec = 0;
// ──────────────────────────────────────────────────────────────────────────────

WiFiClient   espClient;
PubSubClient mqttClient(espClient);
unsigned long lastPublish = 0;

// ──────────────────────────────────────────────────────────────────────────────
void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(wifiName);
  WiFi.begin(wifiName, wifiKey);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());
}

// ──────────────────────────────────────────────────────────────────────────────
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT broker...");
    String clientId = "ESP32Touch-" + String((uint32_t)ESP.getEfuseMac(), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println(" connected!");
    } else {
      Serial.print(" failed. State=");
      Serial.print(mqttClient.state());
      Serial.println(" | Retrying in 2s...");
      delay(2000);
    }
  }
}

// ──────────────────────────────────────────────────────────────────────────────
void initTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.print("Waiting for NTP sync");
  time_t nowSecs = time(nullptr);
  int retries = 0;
  while (nowSecs < 100000 && retries < 20) {
    Serial.print(".");
    delay(500);
    nowSecs = time(nullptr);
    retries++;
  }
  Serial.println();
  Serial.println(nowSecs > 100000 ? "Time synchronized!" : "Time sync failed — using fallback.");
}

// ──────────────────────────────────────────────────────────────────────────────
String getLocalTimeString() {
  time_t nowSecs = time(nullptr);
  struct tm timeinfo;
  localtime_r(&nowSecs, &timeinfo);
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buf);
}

// ──────────────────────────────────────────────────────────────────────────────
void publishTouchJson(bool occupied, long rawValue) {
  String json = "{";
  json += "\"Chair\":" + String(chairId) + ",";
  json += "\"Type\":\"Touch\",";
  json += "\"RawValue\":" + String(rawValue) + ",";
  json += "\"Status\":\"" + String(occupied ? "True" : "False") + "\",";
  json += "\"Time\":\"" + getLocalTimeString() + "\"";
  json += "}";

  bool ok = mqttClient.publish(mqtt_topic, json.c_str());
  Serial.println(ok ? "Published: " + json : "Publish FAILED: " + json);
}

// ──────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(1000);

  connectWiFi();
  initTime();

  // Setup MQTT (setServer only once here)
  mqttClient.setServer(mqtt_server, mqtt_port);

  Serial.println("Touch threshold: " + String(threshold));
  Serial.println("Tip: Watch raw touch values below and adjust threshold if needed.");
}

// ──────────────────────────────────────────────────────────────────────────────
void loop() {
  if (!mqttClient.connected()) reconnectMQTT();
  mqttClient.loop();

  unsigned long now = millis();
  if (now - lastPublish >= PUBLISH_INTERVAL) {
    lastPublish = now;

    long touchVal = touchRead(TOUCH_PIN);
    bool occupied = (touchVal < threshold);

    Serial.print("Touch raw value: ");
    Serial.print(touchVal);
    Serial.println(occupied ? "  → Occupied (True)" : "  → Vacant (False)");

    publishTouchJson(occupied, touchVal);
  }
}
