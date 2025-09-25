// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */

//
// Example to use a Zero-Cross Detection based dimmer (TRIAC or Random SSR)
//
// This example also shows how to auto-detect the grid frequency (50Hz or 60Hz) using MycilaPulseAnalyzer library
//

#include <Arduino.h>
#include <MycilaDimmerZeroCross.h>
#include <MycilaPulseAnalyzer.h>

static Mycila::PulseAnalyzer pulseAnalyzer;
static Mycila::Dimmer* dimmer;

static void initZCD() {
  // Initialize the Zero-Cross Detection (ZCD)
  // You can use MycilaPulseAnalyzer library to detect zero-crossing events from the AC waveform
  // See MycilaPulseAnalyzer documentation for more details
  // MycilaPulseAnalyzer supports many types of ZCD modules (like the Robodyn ZCD or the ZCD from Daniel S., JSY-194g, etc)

  // IMPORTANT NOTE:
  // Mycila Pulse Analyzer library is optional: if you already have a library or code capable of detecting zero-crossing events,
  // you can reuse it and call Mycila::ZeroCrossDimmer::onZeroCross() from your own ISR.

  // Default valu is: -150 us
  // Sends the Zero-Cross even 150 us before the real zero-crossing point of teh voltage
  pulseAnalyzer.setZeroCrossEventShift(-150);

  // Link the ZCD system to our dimmer
  // The dimmer will be notified of zero-crossing events and will trigger the TRIAC/SSR at the right time
  pulseAnalyzer.onZeroCross(Mycila::ZeroCrossDimmer::onZeroCross);

  pulseAnalyzer.begin(35); // GPIO connected to the ZCD output. This can be an input-only pin.
}

static Mycila::Dimmer* createDimmer() {
  Mycila::ZeroCrossDimmer* dimmer = new Mycila::ZeroCrossDimmer();

  // GPIO connected to the dimmer control pin (or Vcc of random SSR)
  dimmer->setPin(GPIO_NUM_25);

  return dimmer;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  initZCD();

  dimmer = createDimmer();

  while (!pulseAnalyzer.getNominalGridSemiPeriod()) {
    Serial.printf("Waiting for grid frequency detection...\n");
    delay(200);
  }

  Serial.printf("Grid frequency detected: %d Hz\n", pulseAnalyzer.getNominalGridFrequency());
  dimmer->setSemiPeriod(pulseAnalyzer.getNominalGridSemiPeriod());

  dimmer->begin();

  Serial.printf("\nProgressive dimming...\n");

  dimmer->setDutyCycleLimit(1);
  dimmer->setDutyCycleMin(0);
  dimmer->setDutyCycleMax(1);
  dimmer->setDutyCycle(0);

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
  pulseAnalyzer.end();

  dimmer = nullptr;
  delete dimmer;
}

void loop() {
  vTaskDelete(NULL);
}
