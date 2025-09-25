// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */

//
// Example how to use Json output
//

#include <Arduino.h>
#include <ArduinoJson.h>
#include <MycilaDimmer.h>
#include <Wire.h>

static Mycila::Dimmer* createDimmer() {
  Mycila::DFRobotDimmer* dimmer = new Mycila::DFRobotDimmer();
  Wire.begin(SDA, SCL);
  dimmer->setWire(Wire);
  dimmer->setOutput(Mycila::DFRobotDimmer::Output::RANGE_0_10V);
  dimmer->setChannel(1);
  dimmer->setSKU(Mycila::DFRobotDimmer::SKU::DFR0971_GP8403);
  return dimmer;
}

static Mycila::Dimmer* dimmer;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  dimmer = createDimmer();
  dimmer->setSemiPeriod(10000); // 50Hz grid frequency
  dimmer->begin();

  for (int i = 0; i <= 100; i += 10) {
    dimmer->setDutyCycle(i / 100.0f);
    JsonDocument doc;
    dimmer->toJson(doc.to<JsonObject>());
    serializeJson(doc, Serial);
    Serial.println();
    delay(1000);
  }

  dimmer->end();

  dimmer = nullptr;
  delete dimmer;
}

void loop() {
  vTaskDelete(NULL);
}
