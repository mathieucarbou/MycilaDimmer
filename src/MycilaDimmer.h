// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#define MYCILA_DIMMER_VERSION          "2.1.0"
#define MYCILA_DIMMER_VERSION_MAJOR    2
#define MYCILA_DIMMER_VERSION_MINOR    1
#define MYCILA_DIMMER_VERSION_REVISION 0

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

#include <assert.h>
#include <esp32-hal-gpio.h>

namespace Mycila {
  class Dimmer {
    public:
      virtual ~Dimmer() {};

      virtual void begin() = 0;
      virtual void end() = 0;
      virtual const char* type() const = 0;

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

      ///////////////
      // POWER LUT //
      ///////////////

      /**
       * @brief Enable or disable the use of power LUT for dimmer curve
       * The power LUT provides a non-linear dimming curve that is more aligned with human perception of brightness.
       * If disabled, a linear dimming curve will be used.
       * @param enable : true to enable, false to disable
       * @param semiPeriod : the semi-period of the grid frequency in us (10000 for 50Hz, 8333 for 60Hz).
       * @note If false is passed, the semi-period parameter is ignored and the already set semi-period is kept unchanged.
       * @note If true is passed, the function validates that a semi-period is set or already exists:
       * @note - if a semi-period is provided (>0), it will be used
       * @note - if no semi-period is provided (0), the already set semi-period will be used (must be >0)
       * @note - if no semi-period is provided and no semi-period was set, an assertion will fail
       */
      void enablePowerLUT(bool enable, uint16_t semiPeriod = 0) {
        if (!enable) {
          _powerLUTEnabled = false;
          return;
        }
        // Enabling the LUT
        if (semiPeriod > 0) {
          // A semi-period is provided, use it
          _semiPeriod = semiPeriod;
        } else {
          // semiPeriod == 0, use already set semi-period, must be >0
          if (_semiPeriod == 0) {
            ESP_LOGE("MycilaDimmer", "enablePowerLUT: semiPeriod must be provided or must be already set when enabling power LUT");
          }
          assert(_semiPeriod > 0);
        }
        _powerLUTEnabled = true;
      }

      /**
       * @brief Check if the power LUT is enabled
       */
      bool isPowerLUTEnabled() const { return _powerLUTEnabled; }

      /**
       * @brief Get the semi-period in us used for the power LUT calculations. If LUT is disabled, returns 0.
       */
      uint16_t getPowerLUTSemiPeriod() const { return _powerLUTEnabled ? _semiPeriod : 0; }

      ///////////////////
      // DIMMER STATES //
      ///////////////////

      /**
       * @brief Check if the dimmer is enabled (if it was able to initialize correctly)
       */
      bool isEnabled() const { return _enabled; }

      /**
       * @brief Returns true if the dimmer is marked online
       */
      bool isOnline() const { return _enabled && _online; }

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
      bool setDutyCycle(float dutyCycle) {
        // Apply limit and save the wanted duty cycle.
        // It will only be applied when dimmer will be on.
        _dutyCycle = _contrain(dutyCycle, 0, _dutyCycleLimit);

        const float mapped = getDutyCycleMapped();

        if (_powerLUTEnabled) {
          if (mapped == 0) {
            _dutyCycleFire = 0.0f;
          } else if (mapped == 1) {
            _dutyCycleFire = 1.0f;
          } else {
            _dutyCycleFire = 1.0f - static_cast<float>(_lookupFiringDelay(mapped, _semiPeriod)) / static_cast<float>(_semiPeriod);
          }
        } else {
          _dutyCycleFire = mapped;
        }

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
       * @brief Get the real firing duty cycle applied to the dimmer in the range [0, 1]
       * @brief - At 0% power, the ratio is equal to 0.
       * @brief - At 100% power, the ratio is equal to 1.
       * @return The duty cycle applied on the hardware, or 0 if the dimmer is offline
       * The firing ratio represents the actual proportion of time the dimmer is conducting power to the load within each AC cycle.
       * It is computed based on the remapped duty cycle and eventually the power LUT if enabled.
       */
      float getDutyCycleFire() const { return isOnline() ? _dutyCycleFire : 0.0f; }

      ///////////////
      // HARMONICS //
      ///////////////

      // Calculate harmonics based on dimmer firing angle for resistive loads
      // array[0] = H1 (fundamental), array[1] = H3, array[2] = H5, array[3] = H7, etc.
      // Only odd harmonics are calculated (even harmonics are negligible for symmetric dimmers)
      // Returns true if harmonics were calculated, false if dimmer is not active
      bool calculateHarmonics(float* array, size_t n) const {
        if (array == nullptr || n == 0)
          return false;

        // Check if dimmer is active and routing
        if (!isOnline() || _dutyCycleFire <= 0.0f) {
          for (size_t i = 0; i < n; i++) {
            array[i] = 0.0f; // No power, no harmonics
          }
          return true;
        }

        if (_dutyCycleFire >= 1.0f) {
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

        return _calculateHarmonics(array, n);
      }

#ifdef MYCILA_JSON_SUPPORT
      /**
       * @brief Serialize Dimmer information to a JSON object
       *
       * @param root: the JSON object to serialize to
       */
      virtual void toJson(const JsonObject& root) const {
        root["type"] = type();
        root["enabled"] = _enabled;
        root["online"] = _online;
        root["state"] = isOn() ? "on" : "off";
        root["duty_cycle"] = _dutyCycle;
        root["duty_cycle_mapped"] = getDutyCycleMapped();
        root["duty_cycle_fire"] = _dutyCycleFire;
        root["duty_cycle_limit"] = _dutyCycleLimit;
        root["duty_cycle_min"] = _dutyCycleMin;
        root["duty_cycle_max"] = _dutyCycleMax;
        root["power_lut"] = _powerLUTEnabled;
        root["power_lut_semi_period"] = _semiPeriod;
        JsonObject harmonics = root["harmonics"].to<JsonObject>();
        float output[11]; // H1 to H21
        if (calculateHarmonics(output, 11)) {
          for (size_t i = 0; i < 11; i++) {
            if (!std::isnan(output[i])) {
              char key[8];
              snprintf(key, sizeof(key), "H%d", static_cast<int>(2 * i + 1));
              harmonics[key] = output[i];
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

      bool _powerLUTEnabled = false;
      uint16_t _semiPeriod = 0;

      virtual bool _apply() = 0;
      virtual bool _calculateHarmonics(float* array, size_t n) const = 0;

      static uint16_t _lookupFiringDelay(float dutyCycle, uint16_t semiPeriod);

      static inline float _contrain(float amt, float low, float high) {
        return (amt < low) ? low : ((amt > high) ? high : amt);
      }

      static bool _calculatePhaseControlHarmonics(float dutyCycleFire, float* array, size_t n) {
        // getDutyCycleFire() returns the conduction angle normalized (0-1)
        // Convert to firing angle: α = π × (1 - conduction)
        const float firingAngle = M_PI * (1.0f - dutyCycleFire);
        const float sin_2a = sinf(2.0f * firingAngle);

        // RMS of fundamental component: I1_rms = (1/π) × √[2(π - α + ½sin(2α))]
        const float i1_rms = sqrtf((2.0f / M_PI) * (M_PI - firingAngle + 0.5f * sin_2a));

        if (i1_rms <= 0.001f)
          return false;

        array[0] = 100.0f; // H1 (fundamental) = 100% reference

        // Pre-compute constant values
        static constexpr float inv_pi_2 = 2.0f / M_PI;
        static constexpr float inv_sqrt2 = 0.70710678f; // 1/√2
        const float scale_factor = inv_pi_2 * inv_sqrt2 * 100.0f / i1_rms;

        // Calculate odd harmonics (H3, H5, H7, ...)
        // Formula: Hn% = (2/π√2) × |cos((n-1)α)/(n-1) - cos((n+1)α)/(n+1)| / I1_rms × 100
        for (size_t i = 1; i < n; i++) {
          const float n_f = static_cast<float>(2 * i + 1); // 3, 5, 7, 9, ...
          const float n_minus_1 = n_f - 1.0f;
          const float n_plus_1 = n_f + 1.0f;

          // Compute Fourier coefficient
          const float coeff = cosf(n_minus_1 * firingAngle) / n_minus_1 -
                              cosf(n_plus_1 * firingAngle) / n_plus_1;

          // Convert to percentage of fundamental
          array[i] = fabsf(coeff) * scale_factor;
        }

        return true;
      }
  };

  class VirtualDimmer : public Dimmer {
    public:
      virtual ~VirtualDimmer() { end(); }

      virtual void begin() { _enabled = true; }
      virtual void end() { _enabled = false; }
      virtual const char* type() const { return "virtual"; }

    protected:
      virtual bool _apply() { return true; }
      virtual bool _calculateHarmonics(float* array, size_t n) const {
        for (size_t i = 0; i < n; i++) {
          array[i] = 0.0f; // No harmonics for virtual dimmer
        }
        return true;
      }
  };
} // namespace Mycila

#include "MycilaDimmerDFRobot.h"
#include "MycilaDimmerPWM.h"
#include "MycilaDimmerThyristor.h"
