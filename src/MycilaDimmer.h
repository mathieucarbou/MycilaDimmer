// SPDX-License-Identifier: MIT
/*
 * Copyright (C) Mathieu Carbou
 */
#pragma once

#define MYCILA_DIMMER_VERSION          "2.4.0"
#define MYCILA_DIMMER_VERSION_MAJOR    2
#define MYCILA_DIMMER_VERSION_MINOR    4
#define MYCILA_DIMMER_VERSION_REVISION 0

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

      ///////////////////
      // DIMMER CONFIG //
      ///////////////////

      /**
       * @brief Set the power duty cycle limit of the dimmer. The duty cycle will be clamped to this limit.
       *
       * @param limit: the power duty cycle limit in the range [0.0, 1.0]
       */
      void setDutyCycleLimit(float limit) {
        _dutyCycleLimit = _contrain(limit, 0, 1);
        if (_dutyCycle > _dutyCycleLimit)
          setDutyCycle(_dutyCycleLimit);
      }

      /**
       * @brief Duty remapping (equivalent to Shelly Dimmer remapping feature).
       * Useful to calibrate the dimmer when using for example a PWM signal to 0-10V analog convertor connected to a voltage regulator which is only working in a specific voltage range like 1-8V.
       *
       * @param min: Set the new "0" value for the power duty cycle. The duty cycle in the range [0.0, 1.0] will be remapped to [min, max].
       */
      void setDutyCycleMin(float min) {
        _dutyCycleMin = _contrain(min, 0, _dutyCycleMax);
        setDutyCycle(_dutyCycle);
      }

      /**
       * @brief Duty remapping (equivalent to Shelly Dimmer remapping feature).
       * Useful to calibrate the dimmer when using for example a PWM signal to 0-10V analog convertor connected to a voltage regulator which is only working in a specific voltage range like 1-8V.
       *
       * @param max: Set the new "1" value for the power duty cycle. The duty cycle in the range [0.0, 1.0] will be remapped to [min, max].
       */
      void setDutyCycleMax(float max) {
        _dutyCycleMax = _contrain(max, _dutyCycleMin, 1);
        setDutyCycle(_dutyCycle);
      }

      /**
       * @brief Get the power duty cycle limit of the dimmer
       */
      float getDutyCycleLimit() const { return _dutyCycleLimit; }

      /**
       * @brief Get the remapped "0" of the dimmer duty cycle
       */
      float getDutyCycleMin() const { return _dutyCycleMin; }

      /**
       * @brief Get the remapped "1" of the dimmer duty cycle
       */
      float getDutyCycleMax() const { return _dutyCycleMax; }

      /////////////////
      // SEMi-PERIOD //
      /////////////////

      /**
       * @brief Get the semi-period in us used for the power LUT calculations. If LUT is disabled, returns 0.
       */
      static uint16_t getSemiPeriod() { return _semiPeriod; }

      /**
       * @brief Set the semi-period of the grid frequency in us for this dimmer. This is mandatory when using power LUT.
       * @brief Typical values are 10000 for 50Hz and 8333 for 60Hz.
       * @brief The value can also come from MycilaPulseAnalyzer
       */
      static void setSemiPeriod(uint16_t semiPeriod) { _semiPeriod = semiPeriod; }

      ///////////////////
      // DIMMER STATES //
      ///////////////////

      /**
       * @brief Check if the dimmer is enabled (if it was able to initialize correctly)
       */
      bool isEnabled() const { return _enabled; }

      /**
       * @brief Returns true if the dimmer is online
       * @brief A dimmer is considered online if it is enabled, marked online
       */
      virtual bool isOnline() const { return _enabled && _online; }

      /**
       * @brief Set the online status of the dimmer
       * @brief This flag can be used to temporarily disable the dimmer when not connected to the grid
       */
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

      ////////////////////
      // DIMMER CONTROL //
      ////////////////////

      /**
       * @brief Turn on the dimmer at full power
       */
      void on() { setDutyCycle(1); }

      /**
       * @brief Turn off the dimmer
       */
      void off() { setDutyCycle(0); }

      /**
       * @brief Check if the dimmer is off
       */
      bool isOff() const { return !isOn(); }

      /**
       * @brief Check if the dimmer is on
       */
      bool isOn() const { return isOnline() && _dutyCycle; }

      /**
       * @brief Check if the dimmer is on at full power
       */
      bool isOnAtFullPower() const { return _dutyCycle >= _dutyCycleMax; }

      /**
       * @brief Set the power duty
       *
       * @param dutyCycle: the power duty cycle in the range [0.0, 1.0]
       */
      virtual bool setDutyCycle(float dutyCycle) {
        // Apply limit and save the wanted duty cycle.
        // It will only be applied when dimmer will be on.
        _dutyCycle = _contrain(dutyCycle, 0, _dutyCycleLimit);
        _dutyCycleFire = getDutyCycleMapped();
        return isOnline() && _apply();
      }

      ////////////////
      // DUTY CYCLE //
      ////////////////

      /**
       * @brief Get the power duty cycle configured for the dimmer by teh user
       */
      float getDutyCycle() const { return _dutyCycle; }

      /**
       * @brief Get the remapped power duty cycle from the currently user set duty cycle
       */
      float getDutyCycleMapped() const { return _dutyCycleMin + _dutyCycle * (_dutyCycleMax - _dutyCycleMin); }

      /**
       * @brief Get the real firing duty cycle (conduction duty cycle) applied to the dimmer in the range [0, 1]
       * @brief - At 0% power, the ratio is equal to 0.
       * @brief - At 100% power, the ratio is equal to 1.
       * @return The duty cycle applied on the hardware, or 0 if the dimmer is offline
       * The firing ratio represents the actual proportion of time the dimmer is conducting power to the load within each AC cycle.
       * It is computed based on the remapped duty cycle and eventually the power LUT if enabled.
       */
      float getDutyCycleFire() const { return isOnline() ? _dutyCycleFire : 0.0f; }

      /////////////
      // METRICS //
      /////////////

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
      /**
       * @brief Serialize Dimmer information to a JSON object
       *
       * @param root: the JSON object to serialize to
       */
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

      ////////////////////
      // STATIC HELPERS //
      ////////////////////

      static inline float _contrain(float amt, float low, float high) {
        return (amt < low) ? low : ((amt > high) ? high : amt);
      }
  };
} // namespace Mycila
