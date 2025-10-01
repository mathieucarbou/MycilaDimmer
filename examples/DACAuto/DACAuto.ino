// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */

//
// Example to use DfRobot DAC to control a voltage regulator
//

#include <Arduino.h>
#include <MycilaDimmer.h>
#include <Wire.h>

static Mycila::Dimmer* createDimmer() {
  Mycila::DFRobotDimmer* dimmer = new Mycila::DFRobotDimmer();

  Wire.begin(SDA, SCL);
  dimmer->setWire(Wire);

  // choose your DAC output voltage depending on your model
  dimmer->setOutput(Mycila::DFRobotDimmer::Output::RANGE_0_10V);
  // dimmer.setOutput(Mycila::DFRobotDimmer::Output::RANGE_0_5V;

  // choose your DAC channel (some have 2 channels: first channel is 0, second channel is 1)
  // dimmer.setChannel(0);
  dimmer->setChannel(1);

  // set the DAC model:
  dimmer->setSKU(Mycila::DFRobotDimmer::SKU::DFR0971_GP8403);
  // dimmer.setSKU(Mycila::DFRobotDimmer::SKU::DFR1071_GP8211S);
  // dimmer.setSKU(Mycila::DFRobotDimmer::SKU::DFR1073_GP8413);

  return dimmer;
}

static Mycila::Dimmer* dimmer;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  dimmer = createDimmer();

  // Grid semi-period in microseconds (us) must be set correctly for the dimmer to work properly
  dimmer->setSemiPeriod(10000); // 50Hz grid frequency
  // dimmer.setSemiPeriod(8333);  // 60Hz grid frequency

  dimmer->begin();

  Serial.printf("\nProgressive dimming...\n");

  Serial.println("From 0% to 100%...");
  for (int i = 0; i <= 1000; i++) {
    dimmer->setDutyCycle(i / 1000.0f);
    delay(10);
  }
  Serial.println("From 100% to 0%...");
  for (int i = 1000; i >= 0; i--) {
    dimmer->setDutyCycle(i / 1000.0f);
    delay(10);
  }

  Serial.println("Done!");

  dimmer->end();

  dimmer = nullptr;
  delete dimmer;
}

void loop() {
  vTaskDelete(NULL);
}
