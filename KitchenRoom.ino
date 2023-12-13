/*
    THIS IS THE Kitchen and & Dining Area
    ALL CONTENT OF THIS CODE IS ONLY WORKING FOR Kitchen and & Dining Area
*/

#include <ESP8266WiFi.h>
#include <WifiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>


//Hotspot connect
const char *ssid = "thus";
const char *password = "123456789";

//define pin for PIR Sensor
const int pirPin = D0;
bool motionDetected = false;

// define variables for all lights and appliciances
int kitchenCeilingLight = D1;
int bathroomLight = D2;
int electricFan = D3;
int wallSource = D4;

// define push button for the appliances and lights
int kitchenCeilingLightPushButton = D5;
int bathroomLightPushButton = D6;
int electrifanPushButton = D7;
int wallSrcPushButton = D8;

// states of the appliance and lights

bool kitchenCeilingLightState = false;
bool bathroomLightState = false;
bool electricfanState = false;
bool wallsrcState = false;

void setup() {
  
  pinMode(kitchenCeilingLight, OUTPUT);
  pinMode(bathroomLight, OUTPUT);
  pinMode(electricFan, OUTPUT);
  pinMode(wallSource, OUTPUT); 
  pinMode(pirPin, INPUT);
  pinMode(kitchenCeilingLightPushButton, INPUT_PULLUP);
  pinMode(bathroomLightPushButton, INPUT_PULLUP);
  pinMode(electrifanPushButton, INPUT_PULLUP);
  pinMode(wallSrcPushButton, INPUT_PULLUP);

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
  int motionStatus = digitalRead(pirPin);
  
  if (motionStatus == HIGH) {
    Serial.println("Motion detected!");
    motionDetected = true;
  } else {
    Serial.println("No motion detected.");
    motionDetected = false;
  }
  // Read the state of the push button
  if (digitalRead(kitchenCeilingLightPushButton) == LOW) {
    // Button is pressed
    kitchenCeilingLightState = !kitchenCeilingLightState;  // Toggle the LED state
    delay(500);  // Debounce delay
  }
  
  
  DynamicJsonDocument doc(1024);
  
  doc["key"] = "J71YztO4uSYJVfES11N6k8OoDYOawJkcj4dkZ6DjnxCMzHCoriazMeHbgg1cVGMK";
  doc["room_name"] = "Kitchen & Dining Area";

  JsonObject sensor_data = doc.createNestedObject("sensor_data");
  sensor_data["temperature"] = 0;
  sensor_data["humidity"] = 0;
  sensor_data["motion_sensor"] = motionDetected;

  JsonObject devices = doc.createNestedObject("devices");

  JsonArray devices_lights = devices.createNestedArray("lights");

  JsonObject devices_lights_0 = devices_lights.createNestedObject();
  devices_lights_0["name"] = "Kitchen Celing Light";
  devices_lights_0["is_active"] = kitchenCeilingLightState;

  JsonObject devices_lights_1 = devices_lights.createNestedObject();
  devices_lights_1["name"] = "Bathroom Light";
  devices_lights_1["is_active"] = bathroomLightState;

  JsonObject devices_lights_2 = devices_lights.createNestedObject();
  devices_lights_2["name"] = "Electric Fan";
  devices_lights_2["is_active"] = electricfanState;

  JsonArray devices_plugs = devices.createNestedArray("plugs");

  JsonObject devices_plugs_0 = devices_plugs.createNestedObject();
  devices_plugs_0["name"] = "Wall Source";
  devices_plugs_0["is_active"] = wallsrcState;

  

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
      const char *ceilingLightName = lightsDoc["lights"][0]["name"];
      bool ceilingLightStatus = lightsDoc["lights"][0]["is_active"];
      kitchenCeilingLightState = ceilingLightStatus;

      const char *bathroomLightName = lightsDoc["lights"][1]["name"];
      bool bathroomLightStatus = lightsDoc["lights"][1]["is_active"];
      bathroomLightState = bathroomLightStatus;

      const char *electrifanName = lightsDoc["lights"][2]["name"];
      bool electrifanStatus = lightsDoc["lights"][2]["is_active"];
      electricfanState = electrifanStatus;

      const char *wallsrcName = lightsDoc["plugs"][0]["name"];
      bool wallsrcStatus = lightsDoc["plugs"][0]["is_active"];
      wallsrcState = wallsrcStatus;

      //digitalWrite(ledLight, ledState ? HIGH : LOW);  // Control the output pin based on ceiling light status
      
      Serial.println(motionDetected);

    }
    
  } else {
    Serial.print("HTTP POST request failed, error: ");
    Serial.println(httpResponseCode);
  }
  digitalWrite(kitchenCeilingLight, kitchenCeilingLightState ? HIGH : LOW); 
  digitalWrite(bathroomLight, bathroomLightState ? HIGH : LOW); 
  digitalWrite(electricFan, electricfanState ? HIGH : LOW); 
  digitalWrite(wallSource, wallsrcState ? HIGH : LOW); 
  //digitalWrite(ledLight, PUSH_BUTTON ? HIGH : LOW);
  
  http.end();

  delay(3500);  // Adjust the delay based on your requirements
}