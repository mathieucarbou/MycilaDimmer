// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */

//
// Example to use a Zero-Cross Detection based dimmer (TRIAC or Random SSR)
//

#include <Arduino.h>
#include <MycilaDimmer.h>
#include <MycilaPulseAnalyzer.h>

#if defined(CONFIG_IDF_TARGET_ESP32)
  #define GPIO_DIMMER GPIO_NUM_25
  #define GPIO_ZCD    GPIO_NUM_35
#else
  #define GPIO_DIMMER GPIO_NUM_20
  #define GPIO_ZCD    GPIO_NUM_8
#endif

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

  pulseAnalyzer.begin(GPIO_ZCD); // GPIO connected to the ZCD output. This can be an input-only pin.
}

static Mycila::Dimmer* createDimmer() {
  Mycila::ZeroCrossDimmer* dimmer = new Mycila::ZeroCrossDimmer();

  // GPIO connected to the dimmer control pin (or Vcc of random SSR)
  dimmer->setPin(GPIO_DIMMER);

  return dimmer;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  initZCD();

  dimmer = createDimmer();

  // Grid semi-period in microseconds (us) must be set correctly for the dimmer to work properly
  dimmer->setSemiPeriod(10000); // 50Hz grid frequency
  // dimmer.setSemiPeriod(8333);  // 60Hz grid frequency

  dimmer->begin();

  Serial.printf("\nConfig:\n");
  Serial.printf(" - Limit: %d %%\n", static_cast<int>(dimmer->getDutyCycleLimit() * 100));
  Serial.printf(" - Remapping: 0 %% => %d %% - 100 %% => %d %%\n", static_cast<int>(dimmer->getDutyCycleMin() * 100), static_cast<int>(dimmer->getDutyCycleMax() * 100));
  Serial.printf(" - Semi-period: %" PRIu16 " us\n", dimmer->getSemiPeriod());

  for (int i = 0; i <= 100; i += 10) {
    dimmer->setDutyCycle(i / 100.0f);
    Serial.printf("Power: %d %% => Mapped to: %d %% with firing delay: %" PRIu16 " us\n",
                  static_cast<int>(dimmer->getDutyCycle() * 100),
                  static_cast<int>(dimmer->getMappedDutyCycle() * 100),
                  dimmer->getFiringDelay());
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
  Serial.printf(" - Semi-period: %" PRIu16 " us\n", dimmer->getSemiPeriod());

  for (int i = 0; i <= 100; i += 10) {
    dimmer->setDutyCycle(i / 100.0f);
    Serial.printf("Power: %d %% => Mapped to: %d %% with firing delay: %" PRIu16 " us\n",
                  static_cast<int>(dimmer->getDutyCycle() * 100),
                  static_cast<int>(dimmer->getMappedDutyCycle() * 100),
                  dimmer->getFiringDelay());
    delay(1000);
  }

  Serial.printf("\nProgressive dimming...\n");

  dimmer->setDutyCycleLimit(1);
  dimmer->setDutyCycleMin(0);
  dimmer->setDutyCycleMax(1);
  dimmer->setDutyCycle(0);

  Serial.printf("From 0% to 100%...\n");
  for (int i = 0; i <= 1000; i++) {
    dimmer->setDutyCycle(i / 1000.0f);
    delay(10);
  }
  Serial.printf("From 100% to 0%...\n");
  for (int i = 1000; i >= 0; i--) {
    dimmer->setDutyCycle(i / 1000.0f);
    delay(10);
  }

  Serial.printf("Done!\n");

  dimmer->end();
  pulseAnalyzer.end();

  dimmer = nullptr;
  delete dimmer;
}

void loop() {
  vTaskDelete(NULL);
}
