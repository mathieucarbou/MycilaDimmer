

# File MycilaDimmerDFRobot.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**MycilaDimmerDFRobot.h**](MycilaDimmerDFRobot_8h.md)

[Go to the documentation of this file](MycilaDimmerDFRobot_8h.md)


```C++
// SPDX-License-Identifier: MIT
/*
 * Copyright (C) Mathieu Carbou
 */
#pragma once

#include "MycilaDimmerPhaseControl.h"

#include <Wire.h>

namespace Mycila {
  class DFRobotDimmer : public PhaseControlDimmer {
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

      void setOutput(Output output) { _output = output; }
      Output getOutput() const { return _output; }

      void setDeviceAddress(uint8_t deviceAddress) { _deviceAddress = deviceAddress; }
      uint8_t getDeviceAddress() const { return _deviceAddress; }

      void setChannel(uint8_t channel) { _channel = channel; }
      uint8_t getChannel() const { return _channel; }

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

      bool begin() override;

      void end() override;

      const char* type() const override { return "dfrobot"; }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const override {
        PhaseControlDimmer::toJson(root);
        root["sku"] = _sku == SKU::DFR1071_GP8211S ? "DFR1071_GP8211S" : _sku == SKU::DFR1073_GP8413 ? "DFR1073_GP8413"
                                                                               : _sku == SKU::DFR0971_GP8403 ? "DFR0971_GP8403"
                                                                                                             : "UNKNOWN";
        root["output"] = _output == Output::RANGE_0_5V ? "0-5V" : "0-10V";
        root["i2c_address"] = _deviceAddress;
        root["channel"] = _channel;
        root["resolution"] = getResolution();
      }
#endif

    protected:
      bool _apply() override {
        if (!isOnline()) {
          return _sendDutyCycle(_deviceAddress, 0) == ESP_OK;
        }
        uint16_t duty = getDutyCycleFire() * ((1 << getResolution()) - 1);
        return _sendDutyCycle(_deviceAddress, duty) == ESP_OK;
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
```


