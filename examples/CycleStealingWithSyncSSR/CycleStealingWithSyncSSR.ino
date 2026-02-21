// SPDX-License-Identifier: MIT
/*
 * Copyright (C) Mathieu Carbou
 */

//
// Example to use a Cycle-Stealing dimmer with a standard synchronous Solid State Relay (SSR)
//

#include <Arduino.h>
#include <MycilaDimmers.h>

#if defined(CONFIG_IDF_TARGET_ESP32)
  #define GPIO_DIMMER GPIO_NUM_26
#else
  #define GPIO_DIMMER GPIO_NUM_20
#endif

static Mycila::CycleStealingDimmer dimmer;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  dimmer.setPin(GPIO_DIMMER);

  // Cycle Stealing dimmer requires to set the grid semi-period
  Mycila::Dimmer::setSemiPeriod(10000); // 10ms for 50Hz grid, 8.33ms for 60Hz grid

  dimmer.begin();
  dimmer.setOnline(true);

  Serial.printf("\nProgressive dimming...\n");

  dimmer.setDutyCycleLimit(1);
  dimmer.setDutyCycleMin(0);
  dimmer.setDutyCycleMax(1);
  dimmer.setDutyCycle(0);

  Serial.println("0 => 100");
  for (int i = 0; i <= 1000; i++) {
    dimmer.setDutyCycle(i / 1000.0f);
    delay(10);
  }
  Serial.println("100 => 0");
  for (int i = 1000; i >= 0; i--) {
    dimmer.setDutyCycle(i / 1000.0f);
    delay(10);
  }

  Serial.println("Done!");
}

void loop() {
  while (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command.endsWith("\r"))
      command.remove(command.length() - 1);
    dimmer.setDutyCycle(constrain(command.toFloat(), 0, 1));
    Serial.printf("Duty cycle set to %.2f%%\n", dimmer.getDutyCycle() * 100);
  }
}
