// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#include "MycilaDimmer.h"
#include <driver/gptimer_types.h>

namespace Mycila {
  /**
   * @brief Thyristor (TRIAC) based dimmer implementation for TRIAC and Random SSR dimmers
   */
  class ThyristorDimmer : public Dimmer {
    public:
      virtual ~ThyristorDimmer() { end(); }

      /**
       * @brief Set the GPIO pin to use for the dimmer
       */
      void setPin(gpio_num_t pin) { _pin = pin; }

      /**
       * @brief Get the GPIO pin used for the dimmer
       */
      gpio_num_t getPin() const { return _pin; }

      /**
       * @brief Set the semi-period of the grid frequency in us. It cannot be zero and it is required for proper dimmer operation.
       */
      void setSemiPeriod(uint16_t semiPeriod) {
        if (semiPeriod == 0) {
          ESP_LOGE("ThyristorDimmer", "setSemiPeriod: semiPeriod must be > 0");
        }
        assert(semiPeriod > 0);
        _semiPeriod = semiPeriod;
      }

      /**
       * @brief Get the semi-period of the grid frequency in us
       */
      uint16_t getSemiPeriod() const { return _semiPeriod; }

      /**
       * @brief Get the firing delay in us of the dimmer in the range [0, semi-period]
       * At 0% power, delay is equal to the semi-period: the dimmer is kept off
       * At 100% power, the delay is 0 us: the dimmer is kept on
       * This value is mostly used for TRIAC based dimmers but also in order to derive metrics based on the phase angle
       */
      uint16_t getFiringDelay() const { return _delay > _semiPeriod ? _semiPeriod : _delay; }

      /**
       * @brief Get the phase angle in degrees (°) of the dimmer in the range [0, 180]
       * At 0% power, the phase angle is equal to 180
       * At 100% power, the phase angle is equal to 0
       */
      float getPhaseAngle() const { return _delay >= _semiPeriod ? 180 : 180 * _delay / _semiPeriod; }

      /**
       * @brief Enable a dimmer on a specific GPIO pin
       *
       * @warning Dimmer won't be enabled if pin is invalid
       * @warning Dimmer won't be activated until the ZCD is enabled
       */
      void begin() override;

      /**
       * @brief Disable the dimmer
       *
       * @warning Dimmer won't be destroyed but won't turn on anymore even is a duty cycle is set.
       */
      void end() override;

      const char* type() const override { return "thyristor"; }

      bool calculateMetrics(Metrics& metrics, float gridVoltage, float loadResistance) const override {
        return isEnabled() && _calculatePhaseControlMetrics(metrics, _dutyCycleFire, gridVoltage, loadResistance);
      }

      /**
       * Callback to be called when a zero-crossing event is detected.
       *
       * - When using MycilaPulseAnalyzer library, this callback can be registered like this:
       *
       * pulseAnalyzer.onZeroCross(Mycila::Dimmer::onZeroCross);
       *
       * - When using your own ISR with the RobotDyn ZCD,      you can call this method with delayUntilZero == 200 since the length of the ZCD pulse is about  400 us.
       * - When using your own ISR with the ZCd from Daniel S, you can call this method with delayUntilZero == 550 since the length of the ZCD pulse is about 1100 us.
       */
      static void onZeroCross(int16_t delayUntilZero, void* args);

#ifdef MYCILA_JSON_SUPPORT
      /**
       * @brief Serialize Dimmer information to a JSON object
       *
       * @param root: the JSON object to serialize to
       */
      void toJson(const JsonObject& root) const override {
        Dimmer::toJson(root);
        root["dimmer_pin"] = _pin;
        root["dimmer_semi_period"] = _semiPeriod;
        root["dimmer_firing_delay"] = getFiringDelay();
        root["dimmer_firing_angle"] = getPhaseAngle();
      }
#endif

    protected:
      bool _apply() override {
        if (!_online || !_semiPeriod || _dutyCycleFire == 0) {
          _delay = UINT16_MAX;
          return _enabled;
        }
        if (_dutyCycleFire == 1) {
          _delay = 0;
          return _enabled;
        }
        _delay = (1.0f - _dutyCycleFire) * static_cast<float>(_semiPeriod);
        return _enabled;
      }

      bool _calculateHarmonics(float* array, size_t n) const override {
        return _calculatePhaseControlHarmonics(_dutyCycleFire, array, n);
      }

    private:
      gpio_num_t _pin = GPIO_NUM_NC;
      uint16_t _delay = UINT16_MAX; // this is the next firing delay to apply

      static bool _fireTimerISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t* event, void* arg);
      static void _registerDimmer(Mycila::ThyristorDimmer* dimmer);
      static void _unregisterDimmer(Mycila::ThyristorDimmer* dimmer);
  };
} // namespace Mycila
