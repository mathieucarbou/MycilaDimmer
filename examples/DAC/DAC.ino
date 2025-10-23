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

  // Enable power LUT (Look-Up Table) for better dimming according to human eye perception and real power curve.
  // Grid semi-period in microseconds (us) must be set correctly for the dimmer to work properly
  dimmer->enablePowerLUT(true, 10000); // 50Hz grid frequency
  // dimmer->enablePowerLUT(true, 8333);  // 60Hz grid frequency

  // Start the dimmer and find the DAC on the I2C bus
  dimmer->begin();

  // Switch the dimmer online
  dimmer->setOnline(true);

  Serial.printf("\nConfig:\n");
  Serial.printf(" - Limit: %d %%\n", static_cast<int>(dimmer->getDutyCycleLimit() * 100));
  Serial.printf(" - Remapping: 0 %% => %d %% - 100 %% => %d %%\n", static_cast<int>(dimmer->getDutyCycleMin() * 100), static_cast<int>(dimmer->getDutyCycleMax() * 100));
  Serial.printf(" - Semi-period: %" PRIu16 " us\n", dimmer->getPowerLUTSemiPeriod());

  for (int i = 0; i <= 100; i += 10) {
    dimmer->setDutyCycle(i / 100.0f);
    Serial.printf("Power: %d %% => Mapped to: %d %% => DAC Output: %d %% (%f V)\n",
                  static_cast<int>(dimmer->getDutyCycle() * 100),
                  static_cast<int>(dimmer->getDutyCycleMapped() * 100),
                  static_cast<int>(dimmer->getDutyCycleFire() * 100),
                  dimmer->getDutyCycleFire() * 10);
    delay(1000);
  }

  // You can limit the dimmer output (0-100%)
  dimmer->setDutyCycleLimit(0.8f);

  // You can remap the 0-100% range if your voltage regulator only starts for example at 1-2V and reaches his limit at 9V
  dimmer->setDutyCycleMin(0.05f); // remap: 0% will now start at 5% in reality
  dimmer->setDutyCycleMax(0.95f); // remap: 100% will now match 95% in reality

  Serial.printf("\nConfig:\n");
  Serial.printf(" - Limit: %d %%\n", static_cast<int>(dimmer->getDutyCycleLimit() * 100));
  Serial.printf(" - Remapping: 0 %% => %d %% - 100 %% => %d %%\n", static_cast<int>(dimmer->getDutyCycleMin() * 100), static_cast<int>(dimmer->getDutyCycleMax() * 100));
  Serial.printf(" - Semi-period: %" PRIu16 " us\n", dimmer->getPowerLUTSemiPeriod());

  for (int i = 0; i <= 100; i += 10) {
    dimmer->setDutyCycle(i / 100.0f);
    Serial.printf("Power: %d %% => Mapped to: %d %% => DAC Output: %d %% (%f V)\n",
                  static_cast<int>(dimmer->getDutyCycle() * 100),
                  static_cast<int>(dimmer->getDutyCycleMapped() * 100),
                  static_cast<int>(dimmer->getDutyCycleFire() * 100),
                  dimmer->getDutyCycleFire() * 10);
    delay(1000);
  }

  dimmer->end();

  delete dimmer;
  dimmer = nullptr;
}

void loop() {
  vTaskDelete(NULL);
}
