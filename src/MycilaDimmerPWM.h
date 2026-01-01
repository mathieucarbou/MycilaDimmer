// SPDX-License-Identifier: MIT
/*
 * Copyright (C) Mathieu Carbou
 */
#pragma once

#include "MycilaDimmer.h"

#define MYCILA_DIMMER_PWM_RESOLUTION 12   // 12 bits resolution => 0-4095 watts
#define MYCILA_DIMMER_PWM_FREQUENCY  1000 // 1 kHz

namespace Mycila {
  /**
   * @brief PWM based dimmer implementation for voltage regulators controlled by a PWM signal to 0-10V analog convertor
   */
  class PWMDimmer : public Dimmer {
    public:
      virtual ~PWMDimmer() { end(); }

      /**
       * @brief Set the GPIO pin to use for the dimmer
       */
      void setPin(gpio_num_t pin) { _pin = pin; }

      /**
       * @brief Get the GPIO pin used for the dimmer
       */
      gpio_num_t getPin() const { return _pin; }

      /**
       * @brief Set the PWM frequency in Hz
       */
      void setFrequency(uint32_t frequency) { this->_frequency = frequency; }

      /**
       * @brief Get the PWM frequency in Hz
       */
      uint32_t getFrequency() const { return _frequency; }

      /**
       * @brief Set the PWM resolution in bits
       */
      void setResolution(uint8_t resolution) { this->_resolution = resolution; }

      /**
       * @brief Get the PWM resolution in bits
       */
      uint8_t getResolution() const { return _resolution; }

      /**
       * @brief Enable a dimmer on a specific GPIO pin
       *
       * @warning Dimmer won't be enabled if pin is invalid
       * @warning Dimmer won't be activated until the ZCD is enabled
       */
      bool begin() override;

      /**
       * @brief Disable the dimmer
       *
       * @warning Dimmer won't be destroyed but won't turn on anymore even is a duty cycle is set.
       */
      void end() override;

      const char* type() const override { return "pwm"; }

      bool calculateMetrics(Metrics& metrics, float gridVoltage, float loadResistance) const override {
        return isEnabled() && _calculatePhaseControlMetrics(metrics, _dutyCycleFire, gridVoltage, loadResistance);
      }

#ifdef MYCILA_JSON_SUPPORT
      /**
       * @brief Serialize Dimmer information to a JSON object
       *
       * @param root: the JSON object to serialize to
       */
      void toJson(const JsonObject& root) const override {
        Dimmer::toJson(root);
        root["pin"] = static_cast<int>(_pin);
        root["frequency"] = _frequency;
        root["resolution"] = _resolution;
      }
#endif

    protected:
      bool _apply() override {
        if (!_online) {
          return ledcWrite(_pin, 0);
        }
        uint32_t duty = _dutyCycleFire * ((1 << _resolution) - 1);
        return ledcWrite(_pin, duty);
      }

      bool _calculateHarmonics(float* array, size_t n) const override {
        return _calculatePhaseControlHarmonics(_dutyCycleFire, array, n);
      }

    private:
      gpio_num_t _pin = GPIO_NUM_NC;
      uint32_t _frequency = MYCILA_DIMMER_PWM_FREQUENCY;
      uint8_t _resolution = MYCILA_DIMMER_PWM_RESOLUTION;
  };
} // namespace Mycila
