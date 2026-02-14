// SPDX-License-Identifier: MIT
/*
 * Copyright (C) Mathieu Carbou
 */

//
// Example how to use Json output
//

#include <Arduino.h>
#include <ArduinoJson.h>
#include <MycilaDimmers.h>
#include <Wire.h>

static Mycila::Dimmer* createDimmer() {
  Mycila::DFRobotDimmer* dimmer = new Mycila::DFRobotDimmer();
  Wire.begin(SDA, SCL);
  dimmer->setWire(Wire);
  dimmer->setOutput(Mycila::DFRobotDimmer::Output::RANGE_0_10V);
  dimmer->setChannel(1);
  dimmer->setSKU(Mycila::DFRobotDimmer::SKU::DFR0971_GP8403);
  dimmer->enablePowerLUT(true);
  return dimmer;
}

static Mycila::Dimmer* dimmer;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  dimmer = createDimmer();
  dimmer->setDutyCycleMin(0.05f); // remap 5% to 95%
  dimmer->setDutyCycleMax(0.95f);
  Mycila::Dimmer::setSemiPeriod(10000); // 50Hz grid frequency
  // Mycila::Dimmer::setSemiPeriod(8333);  // 60Hz grid frequency
  dimmer->begin();
  dimmer->setOnline(true);

  for (int i = 0; i <= 100; i += 10) {
    dimmer->setDutyCycle(i / 100.0f);
    JsonDocument doc;
    dimmer->toJson(doc.to<JsonObject>());
    serializeJson(doc, Serial);
    Serial.println();
    delay(2000);
  }

  dimmer->end();

  delete dimmer;
  dimmer = nullptr;
}

void loop() {
  vTaskDelete(NULL);
}
