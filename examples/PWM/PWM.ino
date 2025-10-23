// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */

//
// Example to use a PWM module to control a dimmer: Analog Converter such as a 3.3V PWM Signal to 0-10V Convertor
//

#include <Arduino.h>
#include <MycilaDimmer.h>

#if defined(CONFIG_IDF_TARGET_ESP32)
  #define GPIO_DIMMER GPIO_NUM_25
#else
  #define GPIO_DIMMER GPIO_NUM_20
#endif

static Mycila::Dimmer* createDimmer() {
  Mycila::PWMDimmer* dimmer = new Mycila::PWMDimmer();

  // GPIO connected to the PWM input of the voltage regulator, or PWM to Analog Converter such as a 3.3V PWM Signal to 0-10V Convertor
  dimmer->setPin(GPIO_DIMMER);

  // PWM frequency, to adjust depending on the hardware used
  dimmer->setFrequency(1000); // 1 kHz

  // PWM resolution: for a dimmer, 12 bits (0-4095) is enough to precisely handle a water heater for example of 4000W
  dimmer->setResolution(12);

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

  // Start the PWM
  dimmer->begin();

  // Switch the dimmer online
  dimmer->setOnline(true);

  Serial.printf("\nConfig:\n");
  Serial.printf(" - Limit: %d %%\n", static_cast<int>(dimmer->getDutyCycleLimit() * 100));
  Serial.printf(" - Remapping: 0 %% => %d %% - 100 %% => %d %%\n", static_cast<int>(dimmer->getDutyCycleMin() * 100), static_cast<int>(dimmer->getDutyCycleMax() * 100));
  Serial.printf(" - Semi-period: %" PRIu16 " us\n", dimmer->getPowerLUTSemiPeriod());

  for (int i = 0; i <= 100; i += 10) {
    dimmer->setDutyCycle(i / 100.0f);
    Serial.printf("Power: %d %% => Mapped to: %d %% => PWM Output: %d %%\n",
                  static_cast<int>(dimmer->getDutyCycle() * 100),
                  static_cast<int>(dimmer->getDutyCycleMapped() * 100),
                  static_cast<int>(dimmer->getDutyCycleFire() * 100));
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
    Serial.printf("Power: %d %% => Mapped to: %d %% => PWM Output: %d %%\n",
                  static_cast<int>(dimmer->getDutyCycle() * 100),
                  static_cast<int>(dimmer->getDutyCycleMapped() * 100),
                  static_cast<int>(dimmer->getDutyCycleFire() * 100));
    delay(1000);
  }

  dimmer->end();

  delete dimmer;
  dimmer = nullptr;
}

void loop() {
  vTaskDelete(NULL);
}
