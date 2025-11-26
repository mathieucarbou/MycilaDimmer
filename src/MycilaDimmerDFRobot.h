// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#include "MycilaDimmer.h"

#include <Wire.h>

namespace Mycila {
  /**
   * @brief DFRobot DFR1071/DFR1073/DFR0971 I2C controlled 0-10V/0-5V dimmer implementation for voltage regulators controlled by a 0-10V/0-5V analog signal
   */
  class DFRobotDimmer : public Dimmer {
    public:
      enum class SKU {
        UNKNOWN,
        // 0-5V/10V output, 1-channel, I2C, 15-bit resolution, 99.99% accuracy
        DFR1071_GP8211S,
        // 0-5V/10V output, 2-channel, I2C, 15-bit resolution, 99.99% accuracy
        DFR1073_GP8413,
        // 0-5V/10V output, 2-channel, I2C, 12-bit resolution, 99.90% accuracy
        DFR0971_GP8403,
      };

      enum class Output {
        RANGE_0_5V,
        RANGE_0_10V,
      };

      virtual ~DFRobotDimmer() { end(); }

      void setWire(TwoWire& wire) { _wire = &wire; }
      TwoWire& getWire() const { return *_wire; }

      void setSKU(SKU sku) { _sku = sku; }
      SKU getSKU() const { return _sku; }

      /**
       * @brief Set output mode of the device: 0-5V or 0-10V
       * @param output output mode
       */
      void setOutput(Output output) { _output = output; }
      Output getOutput() const { return _output; }

      /**
       * @brief I2C address of the device
       * @param deviceAddress I2C address
       */
      void setDeviceAddress(uint8_t deviceAddress) { _deviceAddress = deviceAddress; }
      uint8_t getDeviceAddress() const { return _deviceAddress; }

      /**
       * @brief Set channel number
       * @param channel output channel
       * @n 0: Channel 0 (valid when PWM0 output is configured)
       * @n 1: Channel 1 (valid when PWM1 output is configured)
       * @n 2: All channels (valid when configuring dual channel output)
       */
      void setChannel(uint8_t channel) { _channel = channel; }
      uint8_t getChannel() const { return _channel; }

      /**
       * @brief Get the PWM resolution in bits
       */
      uint8_t getResolution() const {
        switch (_sku) {
          case SKU::DFR1071_GP8211S:
            return 15;
          case SKU::DFR1073_GP8413:
            return 15;
          case SKU::DFR0971_GP8403:
            return 12;
          default:
            return 0;
        }
      }

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

      const char* type() const override { return "dfrobot"; }

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
        root["dfrobot_sku"] = _sku == SKU::DFR1071_GP8211S ? "DFR1071_GP8211S" : _sku == SKU::DFR1073_GP8413 ? "DFR1073_GP8413"
                                                                               : _sku == SKU::DFR0971_GP8403 ? "DFR0971_GP8403"
                                                                                                             : "UNKNOWN";
        root["dfrobot_output"] = _output == Output::RANGE_0_5V ? "0-5V" : "0-10V";
        root["dfrobot_i2c_address"] = _deviceAddress;
        root["dfrobot_channel"] = _channel;
        root["dfrobot_resolution"] = getResolution();
      }
#endif

    protected:
      bool _apply() override {
        if (!_online) {
          return _sendDutyCycle(_deviceAddress, 0) == ESP_OK;
        }
        uint16_t duty = _dutyCycleFire * ((1 << getResolution()) - 1);
        return _sendDutyCycle(_deviceAddress, duty) == ESP_OK;
      }

      bool _calculateHarmonics(float* array, size_t n) const override {
        return _calculatePhaseControlHarmonics(_dutyCycleFire, array, n);
      }

    private:
      SKU _sku = SKU::UNKNOWN;
      Output _output = Output::RANGE_0_10V;
      TwoWire* _wire = &Wire;
      uint8_t _deviceAddress;
      uint8_t _channel = 0;

      uint8_t _sendDutyCycle(uint8_t address, uint16_t duty);
      uint8_t _sendOutput(uint8_t address, Output output);
      uint8_t _send(uint8_t address, uint8_t reg, uint8_t* buffer, size_t size);
      uint8_t _test(uint8_t address);
  };
} // namespace Mycila
