# MycilaDimmer

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Continuous Integration](https://github.com/mathieucarbou/MycilaDimmer/actions/workflows/ci.yml/badge.svg)](https://github.com/mathieucarbou/MycilaDimmer/actions/workflows/ci.yml)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/mathieucarbou/library/MycilaDimmer.svg)](https://registry.platformio.org/libraries/mathieucarbou/MycilaDimmer)

A comprehensive ESP32/Arduino library for controlling AC power devices including TRIACs, SSRs, and voltage regulators through multiple dimming methods.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware Support](#hardware-support)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Dimmer Types](#dimmer-types)
  - [Zero-Cross Dimmer](#zero-cross-dimmer)
  - [PWM Dimmer](#pwm-dimmer)
  - [DFRobot DAC Dimmer](#dfrobot-dac-dimmer)
- [API Reference](#api-reference)
- [Configuration](#configuration)
- [Examples](#examples)
- [Build Configuration](#build-configuration)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## Overview

MycilaDimmer provides a unified interface for controlling AC power devices through different hardware implementations. The library uses a polymorphic architecture that allows you to switch between different dimming methods without changing your application code.

**Key Benefits:**

- **Unified API** - Same interface for all dimmer types
- **IRAM Safe** - Interrupt handlers work during flash operations
- **Hardware Agnostic** - Supports multiple hardware approaches
- **Production Ready** - Used in [YaSolR](https://yasolr.carbou.me) Solar Router

## Features

- 🎛️ **Multiple Control Methods**: Zero-cross detection, PWM, and I2C DAC
- ⚡ **High Performance**: IRAM-safe interrupt handlers with lookup table optimization
- 🔧 **Flexible Configuration**: Duty cycle remapping and calibration support
- 📊 **Rich Telemetry**: Phase angles, firing delays, and power measurements
- 🛡️ **Safety Features**: Duty cycle limits and grid connection detection
- 📱 **JSON Integration**: Optional ArduinoJson support for telemetry
- 🔄 **Real-time Control**: Microsecond-precision timing control

## Hardware Support

### Supported Platforms

- **ESP32** (all variants: ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6, ESP32-H2)
- **Arduino Framework** via PlatformIO or Arduino IDE

### Supported Hardware

- **TRIACs** with zero-cross detection circuits
- **Random Solid State Relays (SSRs)**
- **Voltage Regulators** with 0-10V control (LSA, LCTC, etc)
- **DFRobot DAC Modules** (GP8211S, GP8413, GP8403)

## Installation

### PlatformIO

```ini
[env:myproject]
lib_deps =
    mathieucarbou/MycilaDimmer
```

### Arduino IDE

1. Go to **Sketch** → **Include Library** → **Manage Libraries**
2. Search for "MycilaDimmer"
3. Install the library by Mathieu Carbou

## Quick Start

```cpp
#include <MycilaDimmer.h>

// Create a PWM dimmer instance
Mycila::PWMDimmer dimmer;

void setup() {
  Serial.begin(115200);

  // Configure the dimmer
  dimmer.setPin(GPIO_NUM_26);
  dimmer.begin();

  // Set 50% power
  dimmer.setDutyCycle(0.5);
}

void loop() {
  // Gradually increase power
  for (float power = 0.0; power <= 1.0; power += 0.1) {
    dimmer.setDutyCycle(power);
    delay(1000);
  }
}
```

## Dimmer Types

### Zero-Cross Dimmer

Perfect for TRIAC and Random SSR control with precise phase angle control.

```cpp
#include <MycilaDimmer.h>

Mycila::ZeroCrossDimmer dimmer;

void setup() {
  dimmer.setPin(GPIO_NUM_26);
  dimmer.setSemiPeriod(10000); // 50Hz AC (10ms semi-period)
  dimmer.begin();

  // Register with external zero-cross detector
  pulseAnalyzer.onZeroCross(Mycila::ZeroCrossDimmer::onZeroCross);
}
```

**Features:**

- Microsecond-precision phase angle control
- Lookup table with linear interpolation
- IRAM-safe interrupt handlers
- Supports several zero-cross detection circuits:
  - [Zero-Cross Detector](https://www.pcbway.com/project/shareproject/Zero_Cross_Detector_a707a878.html) from Daniel S
  - [JSY-MK-194G](https://www.jsypowermeter.com/jsy-mk-194g-ac-single-phase-bidirectional-power-energy-meter-module-with-2-split-core-cts-product/)
  - RobotDyn ZCD
  - BM1Z102FJ
  - Any other ZCD circuit providing a clean digital pulse at each zero-crossing

The Zero-Cross Detector mounted on DIN Rail mount from Daniel is is very good and reliable.
I sometimes buy it in bulk from PCBWay.
If you want one, you can have a look at the [YaSolR Pro page](https://yasolr.carbou.me/pro#available-hardware) for the stock status.

### PWM Dimmer

Standard PWM output for SSRs and other PWM-controlled devices.

```cpp
#include <MycilaDimmer.h>

Mycila::PWMDimmer dimmer;

void setup() {
  dimmer.setPin(GPIO_NUM_26);
  dimmer.setFrequency(1000);  // 1kHz PWM
  dimmer.setResolution(12);   // 12-bit resolution
  dimmer.begin();
}
```

**Features:**

- Configurable frequency (default: 1kHz)
- Configurable resolution (default: 12-bit)
- Automatic PWM channel management

For example, this dimmer can be used with a [3.3V to 0-5V/0-10V signal converter](https://aliexpress.com/item/1005004859012736.html)

### DFRobot DAC Dimmer

I2C-based control for DFRobot DAC modules.
Perfect for voltage regulator controlled ([LSA](https://aliexpress.com/item/32606780994.html), [LCTC](https://aliexpress.com/item/1005005008018888.html), etc) with a 0-10V input.

```cpp
#include <MycilaDimmer.h>

Mycila::DFRobotDimmer dimmer;

void setup() {
  dimmer.setSKU(Mycila::DFRobotDimmer::SKU::DFR1071_GP8211S);
  dimmer.setDeviceAddress(0x59);
  dimmer.setOutput(Mycila::DFRobotDimmer::Output::RANGE_0_10V);
  dimmer.begin();
}
```

**Supported Models:**

- **[DFR0971](https://www.dfrobot.com/product-2613.html) (GP8403)**: 12-bit, dual channel, 0-5V/10V
- **[DFR1071](https://www.dfrobot.com/product-2757.html) (GP8211S)**: 15-bit, single channel, 0-5V/10V
- **[DFR1073](https://www.dfrobot.com/product-2756.html) (GP8413)**: 15-bit, dual channel, 0-5V/10V

Here is a [comparison table](https://www.dfrobot.com/blog-13458.html) of the different DFRobot DAC modules.

## API Reference

### Core Methods

```cpp
// Power Control
void on();                          // Full power
void off();                         // Turn off
bool setDutyCycle(float dutyCycle); // Set power (0.0-1.0)

// Status
bool isEnabled();                   // Is configured
bool isOnline();                    // Ready for operation
bool isOn();                        // Currently active
float getDutyCycle();               // Current power setting

// Calibration
void setDutyCycleMin(float min);    // Remap 0% point
void setDutyCycleMax(float max);    // Remap 100% point
void setDutyCycleLimit(float limit); // Safety limit

// Measurements
float getPhaseAngle();              // Phase angle (0-180°)
uint16_t getFiringDelay();          // Delay in microseconds
float getFiringRatio();             // On-time ratio
```

### Advanced Features

```cpp
// Duty Cycle Remapping (Hardware Calibration)
dimmer.setDutyCycleMin(0.1);  // 0% now maps to 10%
dimmer.setDutyCycleMax(0.9);  // 100% now maps to 90%

// Safety Limiting
dimmer.setDutyCycleLimit(0.8); // Never exceed 80% power

// Telemetry (with MYCILA_JSON_SUPPORT)
#ifdef MYCILA_JSON_SUPPORT
JsonDocument doc;
dimmer.toJson(doc.to<JsonObject>());
serializeJson(doc, Serial);
#endif
```

## Configuration

### Build Flags

For **Zero-Cross Dimmer** (IRAM safety):

```ini
build_flags =
  -D CONFIG_ARDUINO_ISR_IRAM=1
  -D CONFIG_GPTIMER_ISR_HANDLER_IN_IRAM=1
  -D CONFIG_GPTIMER_CTRL_FUNC_IN_IRAM=1
  -D CONFIG_GPTIMER_ISR_IRAM_SAFE=1
  -D CONFIG_GPIO_CTRL_FUNC_IN_IRAM=1

lib_deps =
  mathieucarbou/MycilaPulseAnalyzer
```

For **JSON Support**:

```ini
build_flags =
  -D MYCILA_JSON_SUPPORT

lib_deps =
  bblanchon/ArduinoJson
```

## Examples

The library includes comprehensive examples:

- **[DAC Example](examples/DAC/)** - DFRobot DAC module control
- **[JSON Example](examples/Json/)** - Telemetry and JSON integration
- **[PWM Example](examples/PWM/)** - Basic PWM dimming
- **[ZeroCross Example](examples/ZeroCross/)** - TRIAC control with zero-cross detection
- **[ZeroCrossAuto Example](examples/ZeroCrossAuto/)** - Automatic detection of frequency and semi-period
- **[ZeroCrossWithFS Example](examples/ZeroCrossWithFS/)** - File system integration

## Build Configuration

### PlatformIO Configuration

```ini
[platformio]
lib_dir = .
src_dir = examples/ZeroCross  ; or DAC, PWM, etc.

[env]
framework = arduino
board = esp32dev
build_flags =
  -D CONFIG_ARDUHAL_LOG_COLORS
  -D CORE_DEBUG_LEVEL=ARDUHAL_LOG_LEVEL_DEBUG
  -Wall -Wextra
  ; Zero-Cross specific flags
  -D CONFIG_ARDUINO_ISR_IRAM=1
  -D CONFIG_GPTIMER_ISR_HANDLER_IN_IRAM=1
  -D CONFIG_GPTIMER_CTRL_FUNC_IN_IRAM=1
  -D CONFIG_GPTIMER_ISR_IRAM_SAFE=1
  -D CONFIG_GPIO_CTRL_FUNC_IN_IRAM=1
lib_deps =
  bblanchon/ArduinoJson
  mathieucarbou/MycilaPulseAnalyzer
```

### Dependencies

- **ESP32 Arduino Core** (any recent version)
- **[ArduinoJson](https://github.com/bblanchon/ArduinoJson)** (optional, for JSON support)
- **[MycilaPulseAnalyzer](https://github.com/mathieucarbou/MycilaPulseAnalyzer)** (optional, for zero-cross examples)

## Troubleshooting

### Common Issues

**Zero-Cross Dimmer Not Working**

- Ensure IRAM build flags are set
- Check semi-period is configured (`setSemiPeriod()`)
- Verify zero-cross signal is connected and working

**PWM Output Not Visible**

- Check GPIO pin configuration
- Verify PWM frequency and resolution settings
- Use oscilloscope or LED to test output

**DFRobot Module Not Responding**

- Verify I2C wiring (SDA, SCL, power, ground)
- Check device address with I2C scanner
- Ensure correct SKU is configured

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Setup

```bash
git clone https://github.com/mathieucarbou/MycilaDimmer.git
cd MycilaDimmer
pio run
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

**Author**: [Mathieu Carbou](https://github.com/mathieucarbou)  
**Used in**: [YaSolR Solar Router](https://yasolr.carbou.me)
