#include <ota_config.h>

void setup() {
  loadOTA();
  // put your setup code here, to run once:

}

void loop() {
  ArduinoOTA.handle();
  // put your main code here, to run repeatedly:

}
