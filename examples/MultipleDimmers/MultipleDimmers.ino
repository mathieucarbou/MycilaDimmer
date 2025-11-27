// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */

//
// Example to show how to use multiple dimmers
//

#include <Arduino.h>
#include <MycilaDimmer.h>
#include <MycilaPulseAnalyzer.h>

#if defined(CONFIG_IDF_TARGET_ESP32)
  #define GPIO_DIMMER_1 GPIO_NUM_25
  #define GPIO_DIMMER_2 GPIO_NUM_26
  #define GPIO_ZCD      GPIO_NUM_35
#else
  #define GPIO_DIMMER_1 GPIO_NUM_20
  #define GPIO_DIMMER_2 GPIO_NUM_21
  #define GPIO_ZCD      GPIO_NUM_8
#endif

static Mycila::PulseAnalyzer pulseAnalyzer;

static Mycila::ThyristorDimmer dimmer1;
static Mycila::ThyristorDimmer dimmer2;
static Mycila::DFRobotDimmer dimmer3;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  // ZCD for dimmer 1 and 2
  pulseAnalyzer.setZeroCrossEventShift(-150);
  pulseAnalyzer.onZeroCross(Mycila::ThyristorDimmer::onZeroCross);
  pulseAnalyzer.begin(GPIO_ZCD);

  // Dimmer 1 and 2 (ZCD based)
  dimmer1.setPin(GPIO_DIMMER_1);
  dimmer2.setPin(GPIO_DIMMER_2);

  // Dimmer 3 (DfRobot DAC based)
  Wire.begin(SDA, SCL);
  dimmer3.setWire(Wire);
  dimmer3.setOutput(Mycila::DFRobotDimmer::Output::RANGE_0_10V);
  dimmer3.setChannel(1);
  dimmer3.setSKU(Mycila::DFRobotDimmer::SKU::DFR0971_GP8403);

  // Wait until the grid frequency is detected (50Hz or 60Hz)
  while (!pulseAnalyzer.getNominalGridSemiPeriod()) {
    Serial.printf("Waiting for grid frequency detection...\n");
    delay(200);
  }

  // Configure dimmers according to detected grid frequency and use power LUT based dimming
  Serial.printf("Grid frequency detected: %d Hz\n", pulseAnalyzer.getNominalGridFrequency());
  dimmer1.enablePowerLUT(true, pulseAnalyzer.getNominalGridSemiPeriod());
  dimmer2.enablePowerLUT(true, pulseAnalyzer.getNominalGridSemiPeriod());
  dimmer3.enablePowerLUT(true, pulseAnalyzer.getNominalGridSemiPeriod());

  // Start dimmers
  dimmer1.begin();
  dimmer2.begin();
  dimmer3.begin();

  dimmer1.setDutyCycle(0);
  dimmer2.setDutyCycle(0);
  dimmer3.setDutyCycle(0);

  dimmer1.setOnline(true);
  dimmer2.setOnline(true);
  dimmer3.setOnline(true);

  Serial.printf("\nProgressive dimming...\n");

  Serial.println("0 => 100");
  for (int i = 0; i <= 1000; i++) {
    dimmer1.setDutyCycle(i / 1000.0f);
    dimmer2.setDutyCycle(i / 1000.0f);
    dimmer3.setDutyCycle(i / 1000.0f);
    delay(10);
  }
  Serial.println("100 => 0");
  for (int i = 1000; i >= 0; i--) {
    dimmer1.setDutyCycle(i / 1000.0f);
    dimmer2.setDutyCycle(i / 1000.0f);
    dimmer3.setDutyCycle(i / 1000.0f);
    delay(10);
  }

  Serial.println("Done!");

  dimmer1.end();
  dimmer2.end();
  dimmer3.end();
  pulseAnalyzer.end();
}

void loop() {
  vTaskDelete(NULL);
}
