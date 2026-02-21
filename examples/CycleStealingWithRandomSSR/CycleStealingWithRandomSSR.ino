// SPDX-License-Identifier: MIT
/*
 * Copyright (C) Mathieu Carbou
 */

//
// Example to use a Cycle-Stealing dimmer with a Random Solid State Relay (SSR) and Zero-Cross Detection (ZCD) circuit
//

#include <Arduino.h>
#include <MycilaDimmers.h>
#include <MycilaPulseAnalyzer.h>

#if defined(CONFIG_IDF_TARGET_ESP32)
  #define GPIO_DIMMER GPIO_NUM_25
  #define GPIO_ZCD    GPIO_NUM_35
#else
  #define GPIO_DIMMER GPIO_NUM_20
  #define GPIO_ZCD    GPIO_NUM_8
#endif

static Mycila::PulseAnalyzer pulseAnalyzer;
static Mycila::CycleStealingDimmer dimmer;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  // Initialize the Zero-Cross Detection (ZCD)
  // You can use MycilaPulseAnalyzer library to detect zero-crossing events from the AC waveform
  // See MycilaPulseAnalyzer documentation for more details
  // MycilaPulseAnalyzer supports many types of ZCD modules (like the RobotDyn ZCD or the ZCD from Daniel S., JSY-194g, etc)

  // IMPORTANT NOTE:
  // Mycila Pulse Analyzer library is optional: if you already have a library or code capable of detecting zero-crossing events,
  // you can reuse it and call Mycila::CycleStealingDimmer::onZeroCross() from your own ISR.

  // Default valu is: -150 us
  // Sends the Zero-Cross even 150 us before the real zero-crossing point of teh voltage
  pulseAnalyzer.setZeroCrossEventShift(-150);

  // Link the ZCD system to our dimmer
  // The dimmer will be notified of zero-crossing events and will trigger the TRIAC/SSR at the right time
  pulseAnalyzer.onZeroCross(Mycila::CycleStealingDimmer::onZeroCross);

  pulseAnalyzer.begin(GPIO_ZCD); // GPIO connected to the ZCD output. This can be an input-only pin.

  dimmer.setPin(GPIO_DIMMER);

  while (!pulseAnalyzer.getNominalGridSemiPeriod()) {
    Serial.printf("Waiting for grid frequency detection...\n");
    delay(200);
  }

  Serial.printf("Grid frequency detected: %d Hz\n", pulseAnalyzer.getNominalGridFrequency());
  Mycila::Dimmer::setSemiPeriod(pulseAnalyzer.getNominalGridSemiPeriod());

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
