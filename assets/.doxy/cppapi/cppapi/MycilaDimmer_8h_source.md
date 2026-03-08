

# File MycilaDimmer.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**MycilaDimmer.h**](MycilaDimmer_8h.md)

[Go to the documentation of this file](MycilaDimmer_8h.md)


```C++
// SPDX-License-Identifier: MIT
/*
 * Copyright (C) Mathieu Carbou
 */
#pragma once

#define MYCILA_DIMMER_VERSION          "3.0.1"
#define MYCILA_DIMMER_VERSION_MAJOR    3
#define MYCILA_DIMMER_VERSION_MINOR    0
#define MYCILA_DIMMER_VERSION_REVISION 1

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

#include <assert.h>
#include <esp32-hal-gpio.h>

#include <cmath>
#include <cstdio>

namespace Mycila {
  class Dimmer {
    public:
      typedef struct {
          // Output voltage (dimmed)
          float voltage = 0.0f;
          float current = 0.0f;
          float power = 0.0f;
          float apparentPower = 0.0f;
          float powerFactor = NAN;
          float thdi = NAN;
      } Metrics;

    public:
      virtual ~Dimmer() { end(); };

      virtual bool begin() {
        _enabled = true;
        return true;
      }
      virtual void end() { _enabled = false; }
      virtual const char* type() const { return "virtual"; }

      // DIMMER CONFIG //

      void setDutyCycleLimit(float limit) {
        _dutyCycleLimit = _contrain(limit, 0, 1);
        if (_dutyCycle > _dutyCycleLimit)
          setDutyCycle(_dutyCycleLimit);
      }

      void setDutyCycleMin(float min) {
        _dutyCycleMin = _contrain(min, 0, _dutyCycleMax);
        setDutyCycle(_dutyCycle);
      }

      void setDutyCycleMax(float max) {
        _dutyCycleMax = _contrain(max, _dutyCycleMin, 1);
        setDutyCycle(_dutyCycle);
      }

      float getDutyCycleLimit() const { return _dutyCycleLimit; }

      float getDutyCycleMin() const { return _dutyCycleMin; }

      float getDutyCycleMax() const { return _dutyCycleMax; }

      // SEMi-PERIOD //

      static uint16_t getSemiPeriod() { return _semiPeriod; }

      static void setSemiPeriod(uint16_t semiPeriod) { _semiPeriod = semiPeriod; }

      // DIMMER STATES //

      bool isEnabled() const { return _enabled; }

      virtual bool isOnline() const { return _enabled && _online; }

      void setOnline(bool online) {
        _online = online;
        if (!_online) {
          _dutyCycleFire = 0.0f;
          if (_enabled)
            _apply();
        } else {
          setDutyCycle(_dutyCycle);
        }
      }

      // DIMMER CONTROL //

      void on() { setDutyCycle(1); }

      void off() { setDutyCycle(0); }

      bool isOff() const { return !isOn(); }

      bool isOn() const { return isOnline() && _dutyCycle; }

      bool isOnAtFullPower() const { return _dutyCycle >= _dutyCycleMax; }

      virtual bool setDutyCycle(float dutyCycle) {
        // Apply limit and save the wanted duty cycle.
        // It will only be applied when dimmer will be on.
        _dutyCycle = _contrain(dutyCycle, 0, _dutyCycleLimit);
        _dutyCycleFire = getDutyCycleMapped();
        return isOnline() && _apply();
      }

      // DUTY CYCLE //

      float getDutyCycle() const { return _dutyCycle; }

      float getDutyCycleMapped() const { return _dutyCycleMin + _dutyCycle * (_dutyCycleMax - _dutyCycleMin); }

      float getDutyCycleFire() const { return isOnline() ? _dutyCycleFire : 0.0f; }

      // METRICS //

      virtual float getPowerRatio() const {
        // For a linear dimmer, the power ratio is directly proportional to the duty cycle
        return getDutyCycleFire();
      }

      // Calculate harmonics based on dimmer firing angle for resistive loads
      // array[0] = H1 (fundamental), array[1] = H3, array[2] = H5, array[3] = H7, etc.
      // Only odd harmonics are calculated (even harmonics are negligible for symmetric dimmers)
      // Returns true if harmonics were calculated, false if dimmer is not active
      bool calculateHarmonics(float* array, size_t n) const {
        if (array == nullptr || n == 0)
          return false;

        float duty = getDutyCycleFire();

        // Check if dimmer is active and routing
        if (duty <= 0.0f) {
          for (size_t i = 0; i < n; i++) {
            array[i] = 0.0f; // No power, no harmonics
          }
          return true;
        }

        if (duty >= 1.0f) {
          array[0] = 100.0f; // H1 (fundamental) = 100% reference
          for (size_t i = 1; i < n; i++) {
            array[i] = 0.0f; // No harmonics at full power
          }
          return true;
        }

        // Initialize all values to NAN
        for (size_t i = 0; i < n; i++) {
          array[i] = NAN;
        }

        return _calculateDimmerHarmonics(array, n);
      }

      bool calculateMetrics(Metrics& metrics, float gridVoltage, float loadResistance) const {
        if (!_enabled || loadResistance <= 0 || gridVoltage <= 0) {
          return false;
        }

        const float powerRatio = getPowerRatio();

        if (powerRatio <= 0) {
          // no power
          metrics.apparentPower = 0.0f;
          metrics.current = 0.0f;
          metrics.power = 0.0f;
          metrics.powerFactor = NAN;
          metrics.thdi = NAN;
          metrics.voltage = 0.0f;
          return true;
        }

        if (powerRatio >= 1) {
          // full power
          const float nominalPower = gridVoltage * gridVoltage / loadResistance;
          metrics.apparentPower = nominalPower;
          metrics.current = gridVoltage / loadResistance;
          metrics.power = nominalPower;
          metrics.powerFactor = 1.0f;
          metrics.thdi = 0.0f;
          metrics.voltage = gridVoltage;
          return true;
        }

        const float nominalPower = gridVoltage * gridVoltage / loadResistance;

        metrics.power = powerRatio * nominalPower;
        metrics.powerFactor = std::sqrt(powerRatio);
        metrics.voltage = metrics.powerFactor * gridVoltage;
        metrics.current = metrics.voltage / loadResistance;
        metrics.apparentPower = gridVoltage * metrics.current;

        // THDi calculation for resistive load:
        // PF = 1 / sqrt(1 + THDi^2) => THDi = sqrt(1/PF^2 - 1)
        metrics.thdi = 100.0f * std::sqrt(1.0f / (metrics.powerFactor * metrics.powerFactor) - 1.0f);

        return true;
      }

#ifdef MYCILA_JSON_SUPPORT
      virtual void toJson(const JsonObject& root) const {
        static const char* H_LEVELS[] = {"H1", "H3", "H5", "H7", "H9", "H11", "H13", "H15", "H17", "H19", "H21"};

        root["type"] = type();
        root["enabled"] = isEnabled();
        root["online"] = isOnline();
        root["state"] = isOn() ? "on" : "off";
        root["semi_period"] = getSemiPeriod();
        root["duty_cycle"] = getDutyCycle();
        root["duty_cycle_mapped"] = getDutyCycleMapped();
        root["duty_cycle_fire"] = getDutyCycleFire();
        root["duty_cycle_limit"] = getDutyCycleLimit();
        root["duty_cycle_min"] = getDutyCycleMin();
        root["duty_cycle_max"] = getDutyCycleMax();
        JsonObject harmonics = root["harmonics"].to<JsonObject>();
        float* output = new float[11]; // H1 to H21
        if (calculateHarmonics(output, 11)) {
          for (size_t i = 0; i < 11; i++) {
            if (!std::isnan(output[i])) {
              harmonics[H_LEVELS[i]] = output[i];
            }
          }
        }
        delete[] output;
      }
#endif

    protected:
      bool _enabled = false;
      bool _online = false;

      float _dutyCycle = 0.0f;
      float _dutyCycleFire = 0.0f;
      float _dutyCycleLimit = 1.0f;
      float _dutyCycleMin = 0.0f;
      float _dutyCycleMax = 1.0f;

      inline static uint16_t _semiPeriod = 0;

      virtual bool _apply() { return _enabled; }

      virtual bool _calculateDimmerHarmonics(float* array, size_t n) const {
        for (size_t i = 0; i < n; i++) {
          array[i] = 0.0f; // No harmonics for default dimmer
        }
        return true;
      }

      // STATIC HELPERS //

      static inline float _contrain(float amt, float low, float high) {
        return (amt < low) ? low : ((amt > high) ? high : amt);
      }
  };
} // namespace Mycila
```


