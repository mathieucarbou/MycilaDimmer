// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */

//
// This example runs a ZCD based dimmer but does at the same time heavy file system operations
// FS operations are disabling cache and can cause interrupts to be delayed if not properly put in IRAM
//
// MycilaDimmer is designed to work in such conditions with the appropriate compile flags (see README)
//

#include <Arduino.h>
#include <LittleFS.h>
#include <MycilaDimmerZeroCross.h>
#include <MycilaPulseAnalyzer.h>

#if defined(CONFIG_IDF_TARGET_ESP32)
  #define GPIO_DIMMER GPIO_NUM_25
  #define GPIO_ZCD    GPIO_NUM_35
#else
  #define GPIO_DIMMER GPIO_NUM_20
  #define GPIO_ZCD    GPIO_NUM_8
#endif

static void start_async_task_doing_fs_operations() {
  LittleFS.begin(true);
  // Start a task doing heavy FS operations every 500 ms
  // This is to simulate a real-world scenario where the main loop does FS operations
  // and to show that MycilaDimmer is designed to work in such conditions
  xTaskCreate([](void*) {
    for (;;) {
      File f = LittleFS.open("/test.txt", FILE_WRITE);
      if (f) {
        f.printf("Hello World! %lu\n", millis());
        f.close();
      }
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
  },
              "fs_task",
              4096,
              NULL,
              1,
              NULL);
}

static Mycila::PulseAnalyzer pulseAnalyzer;
static Mycila::ZeroCrossDimmer dimmer;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  start_async_task_doing_fs_operations();

  pulseAnalyzer.setZeroCrossEventShift(-150);
  pulseAnalyzer.onZeroCross(Mycila::ZeroCrossDimmer::onZeroCross);
  pulseAnalyzer.begin(GPIO_ZCD);

  while (!pulseAnalyzer.getNominalGridSemiPeriod()) {
    Serial.printf("Waiting for grid frequency detection...\n");
    delay(200);
  }

  Serial.printf("Grid frequency detected: %d Hz\n", pulseAnalyzer.getNominalGridFrequency());

  dimmer.setSemiPeriod(pulseAnalyzer.getNominalGridSemiPeriod());
  dimmer.setPin(GPIO_DIMMER);
  dimmer.begin();

  Serial.printf("\nProgressive dimming...\n");

  dimmer.setDutyCycleLimit(1);
  dimmer.setDutyCycleMin(0);
  dimmer.setDutyCycleMax(1);
  dimmer.setDutyCycle(0);

  Serial.println("From 0% to 100%...");
  for (int i = 0; i <= 1000; i++) {
    dimmer.setDutyCycle(i / 1000.0f);
    delay(10);
  }
  Serial.println("From 100% to 0%...");
  for (int i = 1000; i >= 0; i--) {
    dimmer.setDutyCycle(i / 1000.0f);
    delay(10);
  }

  Serial.println("Done!");

  dimmer.end();
  pulseAnalyzer.end();
}

void loop() {
  vTaskDelete(NULL);
}
