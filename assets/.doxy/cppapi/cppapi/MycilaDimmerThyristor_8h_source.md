

# File MycilaDimmerThyristor.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**MycilaDimmerThyristor.h**](MycilaDimmerThyristor_8h.md)

[Go to the documentation of this file](MycilaDimmerThyristor_8h.md)


```C++
// SPDX-License-Identifier: MIT
/*
 * Copyright (C) Mathieu Carbou
 */
#pragma once

#include "MycilaDimmerPhaseControl.h"
#include <driver/gptimer_types.h>

namespace Mycila {
  class ThyristorDimmer : public PhaseControlDimmer {
    public:
      virtual ~ThyristorDimmer() { end(); }

      void setPin(gpio_num_t pin) { _pin = pin; }

      gpio_num_t getPin() const { return _pin; }

      uint16_t getFiringDelay() const { return _delay > _semiPeriod ? _semiPeriod : _delay; }

      float getPhaseAngle() const { return _delay >= _semiPeriod ? 180 : 180 * _delay / _semiPeriod; }

      bool begin() override;

      void end() override;

      const char* type() const override { return "thyristor"; }

      static void onZeroCross(int16_t delayUntilZero, void* args);

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const override {
        PhaseControlDimmer::toJson(root);
        root["pin"] = _pin;
        root["firing_delay"] = getFiringDelay();
        root["firing_angle"] = getPhaseAngle();
      }
#endif

    protected:
      bool _apply() override {
        float duty = getDutyCycleFire();
        if (!isOnline() || duty == 0) {
          _delay = UINT16_MAX;
          return _enabled;
        }
        if (duty == 1) {
          _delay = 0;
          return _enabled;
        }
        _delay = (1.0f - duty) * static_cast<float>(_semiPeriod);
        return _enabled;
      }

    private:
      gpio_num_t _pin = GPIO_NUM_NC;
      uint16_t _delay = UINT16_MAX; // this is the next firing delay to apply
      uint16_t alarm_count = UINT16_MAX; // when to fire the dimmer

      struct RegisteredDimmer {
          ThyristorDimmer* dimmer = nullptr;
          RegisteredDimmer* prev = nullptr;
          RegisteredDimmer* next = nullptr;
      };

      static struct RegisteredDimmer* dimmers;
      static bool _fireTimerISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t* event, void* arg);
      static void _registerDimmer(Mycila::ThyristorDimmer* dimmer);
      static void _unregisterDimmer(Mycila::ThyristorDimmer* dimmer);
  };
} // namespace Mycila
```


