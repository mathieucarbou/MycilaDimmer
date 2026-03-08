

# File MycilaDimmerCycleStealing.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**MycilaDimmerCycleStealing.h**](MycilaDimmerCycleStealing_8h.md)

[Go to the documentation of this file](MycilaDimmerCycleStealing_8h.md)


```C++
// SPDX-License-Identifier: MIT
/*
 * Copyright (C) Mathieu Carbou
 */
#pragma once

#include "MycilaDimmer.h"
#include <driver/gptimer_types.h>

namespace Mycila {
  class CycleStealingDimmer : public Dimmer {
    public:
      virtual ~CycleStealingDimmer() { end(); }

      void setPin(gpio_num_t pin) { _pin = pin; }

      gpio_num_t getPin() const { return _pin; }

      bool begin() override;

      void end() override;

      const char* type() const override { return "cycle-stealing"; }

      static void onZeroCross(int16_t delayUntilZero, void* args);

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const override {
        Dimmer::toJson(root);
        root["pin"] = _pin;
      }
#endif

    protected:
      bool _apply() override;

      bool _calculateDimmerHarmonics(float* array, size_t n) const override {
        // Unlike phase control (which distorts every cycle identically and creates standard odd harmonics like 3rd, 5th, 7th), cycle stealing creates sub-harmonics
        // and inter-harmonics (frequencies below 50/60Hz or between standard multiples).
        // Since the algorithm uses a dynamic Delta-Sigma modulator (Bresenham-like) rather than a fixed pattern length, the "period" of the repetition is not fixed
        // or easily predicted without a complex simulation.
        return false;
      }

    private:
      gpio_num_t _pin = GPIO_NUM_NC;
      uint16_t duty_milli = 0; // Duty cycle scaled 0–1000; updated from _apply() (avoids float in ISR)
      // Cycle stealing state tracking
      bool semi_period_odd = false; // Track odd/even semi-periods for balance
      int32_t density_error = 0;    // Bresenham accumulator, scaled ×1000 (threshold: 1000)
      int8_t dc_balance = 0;        // DC component balance (-1: owes positive, 1: owes negative)

      struct RegisteredDimmer {
          CycleStealingDimmer* dimmer = nullptr;
          RegisteredDimmer* prev = nullptr;
          RegisteredDimmer* next = nullptr;
      };

      static struct RegisteredDimmer* dimmers;
      static bool _fireTimerISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t* event, void* arg);
      static void _registerDimmer(Mycila::CycleStealingDimmer* dimmer);
      static void _unregisterDimmer(Mycila::CycleStealingDimmer* dimmer);
  };
} // namespace Mycila
```


