//AC SETUP
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Coolix.h>  //  replace library based on your AC unit model, check https://github.com/crankyoldgit/IRremoteESP8266

#define AUTO_MODE kCoolixAuto
#define COOL_MODE kCoolixCool
#define DRY_MODE kCoolixDry
#define HEAT_MODE kCoolixHeat
#define FAN_MODE kCoolixFan
#define FAN_AUTO kCoolixFanAuto
#define FAN_MIN kCoolixFanMin
#define FAN_MED kCoolixFanMed
#define FAN_HI kCoolixFanMax

const uint16_t kIrLed = 4;   // ESP8266 GPIO pin to use for IR blaster.
IRCoolixAC ac(kIrLed);   // Library initialization, change it according to the imported library file.
// END AC SETUP

// Uncomment the following line to enable serial debug output
#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#define DEBUG_ESP_PORT Serial
#define NODEBUG_WEBSOCKETS
#define NDEBUG
#endif

#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#endif

#include "SinricPro.h"
#include "SinricProThermostat.h"

#define WIFI_SSID         "VKHOME 2GHz"
#define WIFI_PASS         "nodevicefound"
#define APP_KEY           "c97c182c-3f5a-4370-810a-cca10391b749"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "9a9b5565-0dc3-44a2-ae72-6288b0a56484-af3b6b3d-eb76-4226-b421-5d46ffb1873f"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#define THERMOSTAT_ID     "6086fab1c54b6f1eb0aa4ff9"    // Should look like "5dc1564130xxxxxxxxxxxxxx"
#define BAUD_RATE          115200                     // Change baudrate to your need


float globalTemperature;
bool globalPowerState;

bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("AC %s turned %s\r\n", deviceId.c_str(), state ? "on" : "off");
  globalPowerState = state;
  if (state) {
    blink();
    ac.on();
    ac.setTemp(26);
    ac.send();
  }
  else if (!state) {
    blink();
    ac.off();
    ac.send();
  }
  return true; // request handled properly
}

bool onTargetTemperature(const String &deviceId, float &temperature) {
  Serial.printf("AC %s set temperature to %f\r\n", deviceId.c_str(), temperature);
  globalTemperature = temperature;
  blink();
  ac.setTemp(temperature);
  ac.send();
  return true;
}

bool onAdjustTargetTemperature(const String & deviceId, float &temperatureDelta) {
  globalTemperature += temperatureDelta;  // calculate absolut temperature
  Serial.printf("AC %s changed temperature about %f to %f", deviceId.c_str(), temperatureDelta, globalTemperature);
  temperatureDelta = globalTemperature; // return absolut temperature
  return true;
}

bool onThermostatMode(const String &deviceId, String &mode) {
  Serial.printf("Thermostat %s set to mode %s\r\n", deviceId.c_str(), mode.c_str());
  //  Serial.println(strcmp(mode.c_str(),"AUTO"));
  if (strcmp(mode.c_str(), "AUTO") == 0) {
    ac.setMode(AUTO_MODE);
    ac.send();
    blink();
  }
  else if (strcmp(mode.c_str(), "COOL") == 0) {
    ac.setMode(COOL_MODE);
    ac.send();
    blink();
  }
  else if (strcmp(mode.c_str(), "HEAT") == 0) {
    ac.setMode(HEAT_MODE);
    ac.send();
    blink();
  }
  else if (strcmp(mode.c_str(), "ECO") == 0) {
    ac.setMode(DRY_MODE);
    ac.send();
    blink();
  }
  else if (strcmp(mode.c_str(), "OFF") == 0) {
    ac.setMode(FAN_MODE);
    ac.send();
    blink();
  }
  //  Special cases for Google Home
  else if (strcmp(mode.c_str(), "off") == 0) {
    ac.off();
    ac.send();
    blink();
  }
  else if (strcmp(mode.c_str(), "cool") == 0) {
    ac.setMode(AUTO_MODE);
    ac.send();
    blink();
  }
  else if (strcmp(mode.c_str(), "heat") == 0) {
    ac.setMode(FAN_MODE);
    ac.send();
    blink();
  }
  return true;
}

void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  IPAddress localIP = WiFi.localIP();
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %d.%d.%d.%d\r\n", localIP[0], localIP[1], localIP[2], localIP[3]);
}

void setupSinricPro() {
  SinricProThermostat &myThermostat = SinricPro[THERMOSTAT_ID];
  myThermostat.onPowerState(onPowerState);
  myThermostat.onTargetTemperature(onTargetTemperature);
  myThermostat.onAdjustTargetTemperature(onAdjustTargetTemperature);
  myThermostat.onThermostatMode(onThermostatMode);

  // setup SinricPro
  SinricPro.onConnected([]() {
    Serial.printf("Connected to SinricPro\r\n");
    digitalWrite(LED_BUILTIN, HIGH);
  });
  SinricPro.onDisconnected([]() {
    Serial.printf("Disconnected from SinricPro\r\n");
    digitalWrite(LED_BUILTIN, LOW);
  });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void blink() {
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
  delay(1000);                      // Wait for a second
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
}

void setup() {
  ac.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");
  setupWiFi();
  setupSinricPro();
}

void loop() {
  SinricPro.handle();
}
