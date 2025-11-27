// SPDX-License-Identifier: MIT
/*
 * Copyright (C) Mathieu Carbou
 */
#pragma once

#include "MycilaDimmer.h"

namespace Mycila {
  class PhaseControlDimmer : public Dimmer {
    public:
      virtual ~PhaseControlDimmer() { end(); }

      ///////////////
      // POWER LUT //
      ///////////////

      /**
       * @brief Enable or disable the use of power LUT for this phase-control dimmer
       * The power LUT provides a non-linear dimming curve that is more aligned with human perception of brightness.
       * If disabled, a linear dimming curve will be used.
       * @param enable : true to enable, false to disable
       */
      void enablePowerLUT(bool enable) { _powerLUTEnabled = enable; }

      /**
       * @brief Check if the power LUT is enabled
       */
      bool isPowerLUTEnabled() const { return _powerLUTEnabled; }

      ///////////////////
      // DIMMER STATES //
      ///////////////////

      /**
       * @brief Returns true if the dimmer is online
       * @brief A phase-control dimmer is considered online if it is enabled, marked online, and, if power LUT is enabled, it must have a valid semi-period set.
       */
      bool isOnline() const override { return _enabled && _online && (!_powerLUTEnabled || _semiPeriod > 0); }

      ////////////////////
      // DIMMER CONTROL //
      ////////////////////

      /**
       * @brief Set the power duty, eventually remapped by the power LUT if enabled
       *
       * @param dutyCycle: the power duty cycle in the range [0.0, 1.0]
       */
      bool setDutyCycle(float dutyCycle) override {
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
            _dutyCycleFire = _semiPeriod > 0 ? (1.0f - static_cast<float>(_lookupFiringDelay(mapped)) / static_cast<float>(_semiPeriod)) : mapped;
          }
        } else {
          _dutyCycleFire = mapped;
        }

        return isOnline() && _apply();
      }

      /////////////
      // METRICS //
      /////////////

      float getPowerRatio() const override {
        if (_powerLUTEnabled) {
          // With LUT enabled, the mapped duty cycle IS the linearized power ratio
          return getDutyCycleMapped();
        } else {
          // Without LUT, we have a linear firing time.
          // We calculate real Power Ratio from Phase Angle using the physical formula:
          // P_ratio = d - sin(2 * pi * d) / (2 * pi)
          const float duty = getDutyCycleFire();
          // Optimization: use multiplication by inverse instead of division
          return duty - sinf(2.0f * M_PI * duty) * (1.0f / (2.0f * M_PI));
        }
      }

#ifdef MYCILA_JSON_SUPPORT
      /**
       * @brief Serialize Dimmer information to a JSON object
       *
       * @param root: the JSON object to serialize to
       */
      virtual void toJson(const JsonObject& root) const {
        Dimmer::toJson(root);
        root["power_lut"] = isPowerLUTEnabled();
      }
#endif

    protected:
      bool _powerLUTEnabled = false;

      static uint16_t _lookupFiringDelay(float dutyCycle) {
        uint32_t duty = dutyCycle * FIRING_DELAY_MAX;
        uint32_t slot = duty * FIRING_DELAYS_SCALE + (FIRING_DELAYS_SCALE >> 1);
        uint32_t index = slot >> 16;
        uint32_t a = FIRING_DELAYS[index];
        uint32_t b = FIRING_DELAYS[index + 1];
        uint32_t delay = a - (((a - b) * (slot & 0xffff)) >> 16); // interpolate a b
        return (delay * _semiPeriod) >> 16;
      }

      bool _calculateDimmerHarmonics(float* array, size_t n) const override {
        // getDutyCycleFire() returns the conduction angle normalized (0-1)
        // Convert to firing angle: α = π × (1 - conduction)
        // At 50% power: α ≈ 90° (π/2), which gives maximum harmonics
        const float firingAngle = M_PI * (1.0f - _dutyCycleFire);

        // Calculate RMS of fundamental component (reference)
        // Formula from Thierry Lequeu: I1_rms = (1/π) × √[2(π - α + ½sin(2α))]
        const float sin_2a = sinf(2.0f * firingAngle);
        const float i1_rms = sqrtf((2.0f / M_PI) * (M_PI - firingAngle + 0.5f * sin_2a));

        if (i1_rms <= 0.001f)
          return false;

        array[0] = 100.0f; // H1 (fundamental) = 100% reference

        // Pre-compute scale factor for efficiency
        const float scale_factor = (2.0f / M_PI) * 0.70710678f * 100.0f / i1_rms;

        // Calculate odd harmonics (H3, H5, H7, ...)
        // Formula for phase-controlled resistive loads (IEEE standard):
        // Hn = (2/π√2) × |cos((n-1)α)/(n-1) - cos((n+1)α)/(n+1)| / I1_rms × 100%
        // This gives the correct harmonic magnitudes relative to the fundamental
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

    private:
      static constexpr uint32_t DIMMER_RESOLUTION = 12;
      static constexpr uint32_t FIRING_DELAYS_LEN = 200;
      static constexpr uint32_t FIRING_DELAY_MAX = (1 << DIMMER_RESOLUTION) - 1;
      static constexpr uint32_t FIRING_DELAYS_SCALE = (FIRING_DELAYS_LEN - 1U) * (1UL << (16 - DIMMER_RESOLUTION));

      // clang-format off
      static constexpr uint16_t FIRING_DELAYS[FIRING_DELAYS_LEN] = {
        0xffff, 0xe877, 0xe240, 0xddd9, 0xda51, 0xd74f, 0xd4aa, 0xd248, 0xd01a, 0xce16,
        0xcc34, 0xca6e, 0xc8c0, 0xc728, 0xc5a1, 0xc42b, 0xc2c3, 0xc168, 0xc019, 0xbed3,
        0xbd98, 0xbc65, 0xbb3b, 0xba17, 0xb8fb, 0xb7e5, 0xb6d5, 0xb5ca, 0xb4c5, 0xb3c4,
        0xb2c8, 0xb1d1, 0xb0dd, 0xafed, 0xaf01, 0xae18, 0xad33, 0xac51, 0xab71, 0xaa95,
        0xa9bb, 0xa8e3, 0xa80e, 0xa73b, 0xa66b, 0xa59c, 0xa4d0, 0xa406, 0xa33d, 0xa276,
        0xa1b1, 0xa0ed, 0xa02b, 0x9f6b, 0x9eac, 0x9dee, 0x9d32, 0x9c76, 0x9bbc, 0x9b04,
        0x9a4c, 0x9996, 0x98e0, 0x982b, 0x9778, 0x96c5, 0x9613, 0x9563, 0x94b2, 0x9403,
        0x9354, 0x92a6, 0x91f9, 0x914c, 0x90a0, 0x8ff5, 0x8f4a, 0x8ea0, 0x8df6, 0x8d4d,
        0x8ca4, 0x8bfb, 0x8b53, 0x8aab, 0x8a04, 0x895d, 0x88b6, 0x8810, 0x876a, 0x86c4,
        0x861e, 0x8579, 0x84d3, 0x842e, 0x8389, 0x82e4, 0x823f, 0x819b, 0x80f6, 0x8051,
        0x7fad, 0x7f08, 0x7e63, 0x7dbf, 0x7d1a, 0x7c75, 0x7bd0, 0x7b2b, 0x7a85, 0x79e0,
        0x793a, 0x7894, 0x77ee, 0x7748, 0x76a1, 0x75fa, 0x7553, 0x74ab, 0x7403, 0x735a,
        0x72b1, 0x7208, 0x715e, 0x70b4, 0x7009, 0x6f5e, 0x6eb2, 0x6e05, 0x6d58, 0x6caa,
        0x6bfb, 0x6b4c, 0x6a9b, 0x69eb, 0x6939, 0x6886, 0x67d3, 0x671e, 0x6668, 0x65b2,
        0x64fa, 0x6442, 0x6388, 0x62cc, 0x6210, 0x6152, 0x6093, 0x5fd3, 0x5f11, 0x5e4d,
        0x5d88, 0x5cc1, 0x5bf8, 0x5b2e, 0x5a62, 0x5993, 0x58c3, 0x57f0, 0x571b, 0x5643,
        0x5569, 0x548d, 0x53ad, 0x52cb, 0x51e6, 0x50fd, 0x5011, 0x4f21, 0x4e2d, 0x4d36,
        0x4c3a, 0x4b39, 0x4a34, 0x4929, 0x4819, 0x4703, 0x45e7, 0x44c3, 0x4399, 0x4266,
        0x412b, 0x3fe5, 0x3e96, 0x3d3b, 0x3bd3, 0x3a5d, 0x38d6, 0x373e, 0x3590, 0x33ca,
        0x31e8, 0x2fe4, 0x2db6, 0x2b54, 0x28af, 0x25ad, 0x2225, 0x1dbe, 0x1787, 0x0000
      };
      // clang-format on
  };
} // namespace Mycila
