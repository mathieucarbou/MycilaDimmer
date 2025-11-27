// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#include "MycilaDimmer.h"
#include <driver/gptimer_types.h>

namespace Mycila {
  class CycleStealingDimmer : public Dimmer {
    public:
      virtual ~CycleStealingDimmer() { end(); }

      /**
       * @brief Set the GPIO pin to use for the dimmer
       */
      void setPin(gpio_num_t pin) { _pin = pin; }

      /**
       * @brief Get the GPIO pin used for the dimmer
       */
      gpio_num_t getPin() const { return _pin; }

      /**
       * @brief Enable a dimmer on a specific GPIO pin
       *
       * @warning Dimmer won't be enabled if pin is invalid
       */
      bool begin() override;

      /**
       * @brief Disable the dimmer
       *
       * @warning Dimmer won't be destroyed but won't turn on anymore even is a duty cycle is set.
       */
      void end() override;

      const char* type() const override { return "cycle-stealing"; }

      bool calculateMetrics(Metrics& metrics, float gridVoltage, float loadResistance) const override {
        // return isEnabled() && _calculatePhaseControlMetrics(metrics, _dutyCycleFire, gridVoltage, loadResistance);
        return isEnabled();
      }

      /**
       * Optional: Integration with a Zero-Cross Detection (ZCD) system
       *
       * Callback to be called when a zero-crossing event is detected.
       *
       * This is optional when using standard (sync) Solid State Relays (SSR) that only activate or deactivate when the AC voltage crosses zero.
       * This is usually the behavior of most SSRs available on the market.
       *
       * If you are using a Random Solid State Relay (SSR) or a TRIAC that can be triggered at any point in the AC cycle, then you need to set this callback
       * so that the library knows when to fire and stop the SSR/TRIAC.
       *
       * When using MycilaPulseAnalyzer library, this callback can be registered like this:
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
        root["pin"] = _pin;
      }
#endif

    protected:
      bool _apply() override;

      bool _calculateHarmonics(float* array, size_t n) const override {
        for (size_t i = 0; i < n; i++) {
          array[i] = 0.0f;
        }
        return true;
      }

    private:
      gpio_num_t _pin = GPIO_NUM_NC;

      struct RegisteredDimmer {
          CycleStealingDimmer* dimmer = nullptr;
          RegisteredDimmer* prev = nullptr;
          RegisteredDimmer* next = nullptr;
          // Cycle stealing state tracking
          bool semi_period_odd = false;     // Track odd/even semi-periods for balance
          uint8_t semi_period_counter = 0;  // Counter within the window
          uint8_t semi_period_on_count = 0; // Number of ON semi-periods in current window
      };

      static struct RegisteredDimmer* dimmers;
      static bool _fireTimerISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t* event, void* arg);
      static void _registerDimmer(Mycila::CycleStealingDimmer* dimmer);
      static void _unregisterDimmer(Mycila::CycleStealingDimmer* dimmer);
  };
} // namespace Mycila
