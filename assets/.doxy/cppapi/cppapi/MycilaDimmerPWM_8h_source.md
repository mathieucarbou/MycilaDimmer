

# File MycilaDimmerPWM.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**MycilaDimmerPWM.h**](MycilaDimmerPWM_8h.md)

[Go to the documentation of this file](MycilaDimmerPWM_8h.md)


```C++
// SPDX-License-Identifier: MIT
/*
 * Copyright (C) Mathieu Carbou
 */
#pragma once

#include "MycilaDimmerPhaseControl.h"

#define MYCILA_DIMMER_PWM_RESOLUTION 12   // 12 bits resolution => 0-4095 watts
#define MYCILA_DIMMER_PWM_FREQUENCY  1000 // 1 kHz

namespace Mycila {
  class PWMDimmer : public PhaseControlDimmer {
    public:
      virtual ~PWMDimmer() { end(); }

      void setPin(gpio_num_t pin) { _pin = pin; }

      gpio_num_t getPin() const { return _pin; }

      void setFrequency(uint32_t frequency) { this->_frequency = frequency; }

      uint32_t getFrequency() const { return _frequency; }

      void setResolution(uint8_t resolution) { this->_resolution = resolution; }

      uint8_t getResolution() const { return _resolution; }

      bool begin() override;

      void end() override;

      const char* type() const override { return "pwm"; }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const override {
        PhaseControlDimmer::toJson(root);
        root["pin"] = static_cast<int>(_pin);
        root["frequency"] = _frequency;
        root["resolution"] = _resolution;
      }
#endif

    protected:
      bool _apply() override {
        if (!isOnline()) {
          return ledcWrite(_pin, 0);
        }
        uint32_t duty = getDutyCycleFire() * ((1 << _resolution) - 1);
        return ledcWrite(_pin, duty);
      }

    private:
      gpio_num_t _pin = GPIO_NUM_NC;
      uint32_t _frequency = MYCILA_DIMMER_PWM_FREQUENCY;
      uint8_t _resolution = MYCILA_DIMMER_PWM_RESOLUTION;
  };
} // namespace Mycila
```


