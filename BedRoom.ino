

#include <ESP8266WiFi.h>
#include <WifiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <DHT.h>

#define DHTPIN 5  // GPIO5 on ESP8266 corresponds to D1 on NodeMCU
#define DHTTYPE DHT11


DHT dht(DHTPIN, DHTTYPE);

const char *ssid = "thus";
const char *password = "123456789";
// define variables for all lights and appliciances
int room1Light = D0;
int room2Light = D3;
int room1Plug = D2;
int room2Plug = D4;

// define push button for the appliances and lights
int room1LightgPushButton = D5;
int room2LightPushButton = D7;
int room1PlugPushButton = D6;
int room2PlugPushButton = D8;

// states of the appliance and lights

bool room1LightState = false;
bool room2LightState = false;
bool room1PlugState = false;
bool room2PlugState = false;

void setup() {
  dht.begin();
  pinMode(room1Light, OUTPUT);
  pinMode(room2Light, OUTPUT);
  pinMode(room2Plug, OUTPUT);
  pinMode(room1Plug, OUTPUT); 
  
  pinMode(room1LightgPushButton, INPUT_PULLUP);
  pinMode(room2LightPushButton, INPUT_PULLUP);
  pinMode(room1PlugPushButton, INPUT_PULLUP);
  pinMode(room2PlugPushButton, INPUT_PULLUP);

  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
  }
  Serial.println("Connected to WiFi");
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  
  
  
  // Read the state of the push button
  if (digitalRead(room1LightgPushButton) == LOW) {
    // Button is pressed
    room1LightState = !room1LightState;  // Toggle the LED state
    delay(500);  // Debounce delay
  }
  
  //Red the data from dht11
  float tmp = dht.readTemperature();
  float hmd = dht.readHumidity();
  DynamicJsonDocument doc(1024);
  
  doc["key"] = "2FKH2cq0LlIn2cTI7RAddjqdtdURKThGK4Sox7aQNooYHs2dgH8mlRXJx8ITE47p";
  doc["room_name"] = "Bed Room";

  JsonObject sensor_data = doc.createNestedObject("sensor_data");
  sensor_data["temperature"] = tmp;
  sensor_data["humidity"] = hmd;
  sensor_data["motion_sensor"] = 0;

  JsonObject devices = doc.createNestedObject("devices");

  JsonArray devices_lights = devices.createNestedArray("lights");

  JsonObject devices_lights_0 = devices_lights.createNestedObject();
  devices_lights_0["name"] = "Room 1 Light";
  devices_lights_0["is_active"] = room1LightState;

  // JsonObject devices_lights_1 = devices_lights.createNestedObject();
  // devices_lights_1["name"] = "Room 1 plug";
  // devices_lights_1["is_active"] = room1PlugState;

  JsonObject devices_lights_1 = devices_lights.createNestedObject();
  devices_lights_1["name"] = "Room 2 Light";
  devices_lights_1["is_active"] = room2LightState;

  JsonArray devices_plugs = devices.createNestedArray("plugs");

  JsonObject devices_plugs_0 = devices_plugs.createNestedObject();
  devices_plugs_0["name"] = "Room 1 Plug";
  devices_plugs_0["is_active"] = room1PlugState;

  JsonObject devices_plugs_1 = devices_plugs.createNestedObject();
  devices_plugs_1["name"] = "Room 2 Plug";
  devices_plugs_1["is_active"] = room2PlugState;
  

  String jsonString;
  serializeJson(doc, jsonString);

  WiFiClient client;  // Create a WiFiClient instance
  HTTPClient http;
  
  // Use the WiFiClient instance as an argument for http.begin
  http.begin(client,  "http://node.homesphere.tech/api/post-data");

  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(jsonString);
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    String response = http.getString();
    Serial.print("Server Response: ");
    Serial.println(response);

    // Parse the lights array
    DynamicJsonDocument lightsDoc(1024);
    DeserializationError jsonError = deserializeJson(lightsDoc, response);

    if (jsonError) {
      Serial.print("JSON parsing failed, error: ");
      Serial.println(jsonError.c_str());
    } else {
      //Extract data for the ceiling light
      const char *Room1Light = lightsDoc["lights"][0]["name"];
      bool Room1LightStatus = lightsDoc["lights"][0]["is_active"];
      room1LightState = Room1LightStatus;

      const char *Room2Light = lightsDoc["lights"][1]["name"];
      bool Room2LightStatus = lightsDoc["lights"][1]["is_active"];
      room2LightState = Room2LightStatus;

      // const char *electrifanName = lightsDoc["lights"][2]["name"];
      // bool electrifanStatus = lightsDoc["lights"][2]["is_active"];
      // electricfanState = electrifanStatus;

      const char *Room1Plug = lightsDoc["plugs"][0]["name"];
      bool Room1PlugState = lightsDoc["plugs"][0]["is_active"];
      room1PlugState = Room1PlugState;
      const char *Room2Plug = lightsDoc["plugs"][1]["name"];
      bool Room2PlugState = lightsDoc["plugs"][1]["is_active"];
      room2PlugState = Room2PlugState;
    }
    
  } else {
    Serial.print("HTTP POST request failed, error: ");
    Serial.println(httpResponseCode);
  }
  digitalWrite(room1Light, room1LightState ? HIGH : LOW); 
  digitalWrite(room1Light, room2LightState ? HIGH : LOW); 
  digitalWrite(room1Plug, room1LightState ? HIGH : LOW); 
  digitalWrite(room2Plug, room2LightState ? HIGH : LOW); 
  //digitalWrite(ledLight, PUSH_BUTTON ? HIGH : LOW);
  
  http.end();

  delay(3500);  // Adjust the delay based on your requirements
}
