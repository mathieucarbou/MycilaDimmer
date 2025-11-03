// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <MycilaDimmerPWM.h>

#include <driver/ledc.h>

// logging
#include <esp32-hal-log.h>

#ifndef GPIO_IS_VALID_OUTPUT_GPIO
  #define GPIO_IS_VALID_OUTPUT_GPIO(gpio_num) ((gpio_num >= 0) && \
                                               (((1ULL << (gpio_num)) & SOC_GPIO_VALID_OUTPUT_GPIO_MASK) != 0))
#endif

#ifndef GPIO_IS_VALID_GPIO
  #define GPIO_IS_VALID_GPIO(gpio_num) ((gpio_num >= 0) && \
                                        (((1ULL << (gpio_num)) & SOC_GPIO_VALID_GPIO_MASK) != 0))
#endif

#define TAG "PWMDimmer"

void Mycila::PWMDimmer::begin() {
  if (_enabled)
    return;

  if (!GPIO_IS_VALID_OUTPUT_GPIO(_pin)) {
    ESP_LOGE(TAG, "Disable PWM Dimmer: Invalid pin: %" PRId8, _pin);
    return;
  }

  ESP_LOGI(TAG, "Enable PWM Dimmer on pin %" PRId8, _pin);

  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);

  if (ledcAttach(_pin, _frequency, _resolution) && ledcWrite(_pin, 0)) {
    _enabled = true;
  } else {
    ESP_LOGE(TAG, "Failed to attach ledc driver on pin %" PRId8, _pin);
    return;
  }

  // restart with last saved value
  setDutyCycle(_dutyCycle);
}

void Mycila::PWMDimmer::end() {
  if (!_enabled)
    return;
  _enabled = false;
  _online = false;
  ESP_LOGI(TAG, "Disable PWM Dimmer on pin %" PRId8, _pin);
  _apply();
  ledcDetach(_pin);
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
}
