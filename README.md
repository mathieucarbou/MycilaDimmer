# MycilaDimmer

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Continuous Integration](https://github.com/mathieucarbou/MycilaDimmer/actions/workflows/ci.yml/badge.svg)](https://github.com/mathieucarbou/MycilaDimmer/actions/workflows/ci.yml)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/mathieucarbou/library/MycilaDimmer.svg)](https://registry.platformio.org/libraries/mathieucarbou/MycilaDimmer)

A comprehensive ESP32/Arduino library for controlling AC power devices including TRIACs, SSRs, and voltage regulators through multiple dimming methods.

![](https://mathieu.carbou.me/MycilaDimmer/assets/img/console.png)
![](https://mathieu.carbou.me/MycilaDimmer/assets/img/anim.gif)

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware Support](#hardware-support)
  - [Supported Platforms](#supported-platforms)
  - [Supported Hardware](#supported-hardware)
- [Installation](#installation)
  - [PlatformIO](#platformio)
  - [Arduino IDE](#arduino-ide)
- [Quick Start](#quick-start)
- [Dimmer Types](#dimmer-types)
  - [Zero-Cross Dimmer](#zero-cross-dimmer)
  - [PWM Dimmer](#pwm-dimmer)
  - [DFRobot DAC Dimmer](#dfrobot-dac-dimmer)
- [Understanding AC Dimming Methods](#understanding-ac-dimming-methods)
  - [Dimming Method Comparison](#dimming-method-comparison)
    - [Phase Control](#phase-control)
    - [Burst Fire on Full Period](#burst-fire-on-full-period)
    - [Burst Fire on Semi-Period](#burst-fire-on-semi-period)
    - [Stochastic Burst Fire (Coming Soon)](#stochastic-burst-fire-coming-soon)
  - [Recommendations for Harmonic Mitigation (Phase Control)](#recommendations-for-harmonic-mitigation-phase-control)
  - [Current MycilaDimmer Support](#current-myciladimmer-support)
  - [Choosing the Right Method](#choosing-the-right-method)
- [References and Further Reading](#references-and-further-reading)
- [API Reference](#api-reference)
  - [Common API (All Dimmer Types)](#common-api-all-dimmer-types)
  - [Zero-Cross Dimmer Specific](#zero-cross-dimmer-specific)
  - [PWM Dimmer Specific](#pwm-dimmer-specific)
  - [DFRobot DAC Dimmer Specific](#dfrobot-dac-dimmer-specific)
  - [Advanced Features](#advanced-features)
- [Configuration](#configuration)
  - [Build Flags](#build-flags)
- [Examples](#examples)
- [Build Configuration](#build-configuration)
  - [PlatformIO Configuration](#platformio-configuration)
  - [Dependencies](#dependencies)
- [Troubleshooting](#troubleshooting)
  - [Common Issues](#common-issues)
- [Contributing](#contributing)
  - [Development Setup](#development-setup)
- [License](#license)
- [Disclaimer](#disclaimer)

## Overview

MycilaDimmer provides a unified interface for controlling AC power devices through different hardware implementations. The library uses a polymorphic architecture that allows you to switch between different dimming methods without changing your application code.

**Key Benefits:**

- **Unified API** - Same interface for all dimmer types
- **IRAM Safe** - Interrupt handlers work during flash operations
- **Hardware Agnostic** - Supports multiple hardware approaches
- **Production Ready** - Used in [YaSolR](https://yasolr.carbou.me) Solar Router

## Features

- ✨ **Flicker-Free Dimming**: Progressive dimming without flickering using precise DAC control or zero-cross detection with quality ZCD circuits
- 🎛️ **Multiple Control Methods**: Zero-cross detection, PWM, and I2C DAC
- ⚡ **High Performance**: IRAM-safe interrupt handlers with lookup table optimization
- 💡 **Selectable Dimming Curves**: Choose between linear dimming or Power LUT for perceptual brightness matching - switch at runtime based on your needs
- � **Flexible Configuration**: Duty cycle remapping, calibration, and user-selectable dimming modes
- 📊 **Rich Telemetry**: Duty cycle measurements, firing ratios, and online status
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

Perfect for TRIAC and Random SSR control with **flicker-free progressive dimming**. Achieves smooth, continuous power control through precise phase angle control when paired with quality zero-cross detection circuits.

```cpp
#include <MycilaDimmer.h>

Mycila::ThyristorDimmer dimmer;

void setup() {
  dimmer.setPin(GPIO_NUM_26);
  dimmer.setSemiPeriod(10000); // 50Hz AC (10ms semi-period)
  dimmer.begin();

  // Register with external zero-cross detector
  pulseAnalyzer.onZeroCross(Mycila::ThyristorDimmer::onZeroCross);
}
```

**Features:**

- **Flicker-free dimming**: Smooth progressive control without visible flickering
- Microsecond-precision phase angle control
- Lookup table with linear interpolation for seamless transitions
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

**Flicker-free progressive dimming** using precision I2C DAC modules.
Perfect for voltage regulator controlled devices ([LSA](https://aliexpress.com/item/32606780994.html), [LCTC](https://aliexpress.com/item/1005005008018888.html), etc) with 0-10V input, providing ultra-smooth dimming without any visible flickering thanks to high-resolution DAC output.

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

## Understanding AC Dimming Methods

When controlling AC power devices like TRIACs, SSRs, and voltage regulators, there are several dimming approaches available. Each method has specific characteristics, advantages, and limitations that affect precision, grid compatibility, and regulatory compliance.

### Dimming Method Comparison

#### Phase Control

**How it works:** Controls power by delaying the firing angle within each AC semi-period (twice per cycle). The TRIAC/SSR activates at a specific point in the sine wave, "chopping" the waveform to deliver partial power.

![](https://mathieu.carbou.me/MycilaDimmer/assets/img/measurements/Oscillo_Dimmer_50.jpeg)

**Advantages:**

- ✅ **High Precision**: Can adjust power at each semi-period (every 10ms for 50Hz), enabling watt-level control
- ✅ **Fast Response**: Instant power adjustment with no delay
- ✅ **Accurate Power Control**: With Power LUT, achieves predictable power output matching the desired level
- ✅ **Regulatory Compliant**: Keeps grid balanced (no DC component) when properly implemented under harmonic regulations
- ✅ **Widely Used**: Standard method in commercial dimmers and variable speed drives

**Limitations:**

- ⚠️ **Harmonics**: Generates harmonic distortion, especially at 50% power (90° phase angle)
  - Harmonics are regulated by CEI 61000-3-2 (Class A devices)
  - H3 (3rd harmonic) is the most significant, exceeds limits at 50% dimming with ~1700W nominal load
  - H15 (15th harmonic) first to exceed limits at ~760W nominal load (but less significant than H3)
  - Maximum compliant load: ~800W without mitigation
  - Maximum significant load: ~1700W
- ⚠️ **Mitigation may be required**: May need RC snubbers, proper wiring, load management, or limiter settings

**MycilaDimmer Implementation:**

- All three implementations (ThyristorDimmer, PWMDimmer, DFRobotDimmer) use phase control
- ThyristorDimmer: Direct TRIAC/Random SSR control with zero-cross detection
- PWMDimmer: Generates PWM signal for converter for voltage regulators (LSA, LCTC) which perform internal phase control
- DFRobotDimmer: Outputs 0-10V analog signal to voltage regulators (LSA, LCTC) which perform internal phase control

#### Burst Fire on Full Period

**How it works:** Rapidly switches complete AC cycles on/off (20ms periods for 50Hz). For example, to achieve 50% power, alternates full power for 20ms, then turns off for 20ms, and so on.

![](https://mathieu.carbou.me/MycilaDimmer/assets/img/measurements/burst_fire_20ms.jpeg)

**Advantages:**

- ✅ **No Harmonics**: Preserves complete sine waves, generates minimal harmonic distortion
- ✅ **Simple Implementation**: Basic on/off switching logic
- ✅ **Compatible with Zero-Cross SSRs**: Can use simpler, cheaper SSRs

**Limitations:**

- ❌ **Flickering**: Visible light can flicker and voltage fluctuations can affect nearby devices
  - Caused by sudden high current draw creating voltage drops
  - Can impact micro-inverters and other sensitive electronics
- ❌ **Slow Response**: Limited precision due to coarse time slots
  - Example: 50 slots in 1-second window for 50Hz = 60W resolution for 3000W load
- ❌ **Heat Dissipation**: Rapid switching generates more heat in SSR
- ❌ **Poor Accuracy**: Cannot achieve watt-level control precision
- ❌ **Delayed Corrections**: By the time adjustment is applied, conditions may have changed

#### Burst Fire on Semi-Period

**How it works:** Switches at semi-period level (every 10ms for 50Hz) to double the control slots and improve response time. For example, to achieve 50% power, alternates full power and no power for 20ms, and 10ms, to avoid creating DC components.

![](https://mathieu.carbou.me/MycilaDimmer/assets/img/measurements/burst_fire_10ms.jpeg)

**Advantages:**

- ✅ **Better Resolution**: Twice as many control slots compared to full-period burst fire
- ✅ **Faster Response**: 2x quicker than full-period burst fire

**Limitations:**

- ❌ **All Full-Period Limitations**: Still suffers from flickering, heat, and inaccuracy but to a lesser extent
- ❌ **DC Component**: Critical regulatory violation in some countries
  - Can create dangerous DC components on AC grid (current asymmetry drawn only one side of the waveform for a short time)
  - Can unbalance grid network by drawing current asymmetrically if not properly balanced
  - **Can violate electrical regulation** - this method cannot be used in some countries where regulations forbid DC components on AC grids

#### Stochastic Burst Fire (Coming Soon)

**How it works:** Probabilistic switching at each zero-cross. At each AC cycle, generates a random number (0-100) and compares it to the desired power level percentage. If the random number is lower, the full wave is allowed through; otherwise, it's blocked.

**Advantages:**

- ✅ **No DC Component**: Always switches at zero-cross on full waves, maintains grid balance
- ✅ **Eliminates Periodic Flickering**: Random distribution prevents visible periodic patterns
- ✅ **Minimal EMF Interference**: Zero-cross switching reduces electromagnetic interference
- ✅ **Multi-Channel Safe**: Random switching prevents simultaneous current spikes across channels
- ✅ **Even Distribution**: Over time, produces statistically accurate power output
- ✅ **No Harmonics**: Preserves complete sine waves like traditional burst fire

**Limitations:**

- ⚠️ **Less Precise**: Statistical accuracy over time, not instant watt-level precision
- ⚠️ **Requires More Cycles**: Needs multiple AC cycles to reach target power level
- ⚠️ **Not for Fast-Response Systems**: Better suited for thermal/heating applications with slower dynamics

**Use Cases:**

- Multi-channel heating systems
- Temperature control with PID regulation
- Applications where eliminating flicker is more important than instant precision
- Systems sensitive to EMF interference

### Recommendations for Harmonic Mitigation (Phase Control)

When using phase control, harmonics can be reduced or partially mitigated through several approaches. Examples (but not limited to):

1. **Power Limiter**: Limit dimmer (in example to 40% of nominal load)
2. **Reduced Load**: Use lower wattage resistance (e.g., 1000W @ 53Ω instead of 3000W @ 18Ω)
3. **Proper Wiring**: Minimize cable length, use appropriate wire gauge
4. **Strategic Placement**: Position router close to grid entry/exit point
5. **Stepped Loads**: Use multiple resistances with relays (e.g., 3x 800W elements)
6. **RC Snubbers**: 100Ω 100nF snubbers can help with sensitive equipment

### Current MycilaDimmer Support

**Currently Supported (Phase Control):**

- ✅ ThyristorDimmer - TRIAC/Random SSR with zero-cross detection
- ✅ PWMDimmer - PWM output for voltage regulators (LSA, LCTC)
- ✅ DFRobotDimmer - I2C DAC for voltage regulators (LSA, LCTC)

**Coming Soon:**

- ✅ BurstFireDimmer - Stochastic burst fire for Zero-Cross SSR and TRIAC/Random SSR with zero-cross detection

### Choosing the Right Method

**For Solar Routers & High-Precision Applications:**

- Use **Phase Control** with appropriate harmonic mitigation
- Provides best accuracy and response time

**For Simple On/Off Control:**

- Consider **Zero-Cross SSR** with simple relay control
- Suitable when precise dimming is not required
- No harmonics, but no variable power control

## References and Further Reading

**Harmonics**

- [CEI 61000-3-2 Harmonic Standards](http://crochet.david.online.fr/bep/copie%20serveur/Normes/cei%2061000-3-2.pdf)
- [Etude des harmoniques du courant de ligne](https://www.thierry-lequeu.fr/data/TRIAC.pdf) (Thierry Lequeu)
- [Détection et atténuation des harmoniques](https://fr.electrical-installation.org/frwiki/Détection_et_atténuation_des_harmoniques) (Schneider Electric)
- [Router via TRIAC et "Pollution" du réseau](https://forum-photovoltaique.fr/viewtopic.php?t=60521) (Forum photovoltaïque)
- [HARMONICS: CAUSES, EFFECTS AND MINIMIZATION](https://www.salicru.com/files/pagina/72/278/jn004a01_whitepaper-armonics_%281%29.pdf) (Ramon Pinyol, R&D Product Leader SALICRU)
- [HARMONIQUES ET DEPOLLUTION DU RESEAU ELECTRIQUE](http://archives.univ-biskra.dz/bitstream/123456789/21913/1/BELHADJ%20KHEIRA%20ET%20BOUZIR%20NESSRINE.pdf) (BELHADJ KHEIRA ET BOUZIR NESSRINE)
- [Impact de la pollution harmonique sur les matériels de réseau](https://theses.hal.science/tel-00441877/document) (Wilfried Frelin)

**TRIAC and Thyristors**

- [NEW TRIACS: IS THE SNUBBER CIRCUIT NECESSARY?](https://www.thierry-lequeu.fr/data/AN437.pdf) (Thierry Lequeu)
- [Le triac](https://emrecmic.wordpress.com/2017/02/07/le-triac/)
- [Le gradateur](http://philippe.demerliac.free.fr/RichardK/Graduateur.pdf) (Richard KOWAL)
- [Switching Loads with Arduino and Solid State Relay](https://www.luisllamas.es/en/arduino-solid-state-relay-ssr/)

**Technical docs and algorithms**

- [Learn: PV Diversion](https://docs.openenergymonitor.org/pv-diversion/)
- [Optimized Random Integral Wave AC Control Algorithm for AC heaters](https://tsltd.github.io)
- [Cycle Stealing Control](https://www.renesas.com/en/document/apn/1164-cycle-stealing-control) (Vladimir Veljkovic)

**Solar Router using this library:**

- [YaSolR Overview - Detailed Analysis](https://yasolr.carbou.me/overview)

## API Reference

**Doxygen documentation** is available [here](https://mathieu.carbou.me/MycilaDimmer/api/index.html).

### Common API (All Dimmer Types)

```cpp
// Lifecycle
void begin();                          // Initialize dimmer
void end();                            // Cleanup and disable
const char* type() const;              // Get dimmer type name

// Power Control
void on();                             // Full power
void off();                            // Turn off
bool setDutyCycle(float dutyCycle);    // Set power (0.0-1.0)

// Status & State
bool isEnabled() const;                // Is configured
bool isOnline() const;                 // Ready for operation (enabled + online)
void setOnline(bool online);           // Set online status (grid connection)
bool isOn() const;                     // Currently active (online + duty > 0)
bool isOff() const;                    // Currently inactive
bool isOnAtFullPower() const;          // Check if at max power

// Calibration & Remapping
void setDutyCycleMin(float min);       // Remap 0% point (hardware calibration)
void setDutyCycleMax(float max);       // Remap 100% point
void setDutyCycleLimit(float limit);   // Clamp max allowed power
float getDutyCycle() const;            // Current power setting
float getDutyCycleMapped() const;      // Get mapped/calibrated duty cycle
float getDutyCycleLimit() const;       // Get current limit
float getDutyCycleMin() const;         // Get current min
float getDutyCycleMax() const;         // Get current max

// Dimming Curve (Power LUT)
void enablePowerLUT(bool enable, uint16_t semiPeriod = 0); // Enable/disable perceptual LUT (default: false)
bool isPowerLUTEnabled() const;         // Check if LUT is enabled
uint16_t getPowerLUTSemiPeriod() const; // Get LUT semi-period (us)

// Measurements
float getDutyCycleFire() const;         // Actual firing ratio (0-1)

#ifdef MYCILA_JSON_SUPPORT
void toJson(const JsonObject& root) const; // Serialize to JSON
#endif
```

### Zero-Cross Dimmer Specific

```cpp
void setPin(gpio_num_t pin);           // Set output GPIO pin
void setSemiPeriod(uint16_t semiPeriod); // Set grid semi-period (us)
static void onZeroCross(int16_t delayUntilZero, void* arg); // Zero-cross callback
```

### PWM Dimmer Specific

```cpp
void setPin(gpio_num_t pin);           // Set output GPIO pin
void setFrequency(uint32_t frequency); // Set PWM frequency (default: 1000 Hz)
void setResolution(uint8_t resolution); // Set PWM resolution (default: 12-bit)
```

### DFRobot DAC Dimmer Specific

```cpp
void setWire(TwoWire& wire);           // Set I2C bus (default: Wire)
void setSKU(SKU sku);                  // Set module SKU (DFR0971/1071/1073)
void setOutput(Output output);         // Set voltage range (0-5V or 0-10V)
void setDeviceAddress(uint8_t addr);   // Set I2C address (default: 0x58)
void setChannel(uint8_t channel);      // Set DAC channel (0 or 1 for dual-channel)
```

### Advanced Features

```cpp
// Duty Cycle Remapping (Hardware Calibration)
dimmer.setDutyCycleMin(0.1);  // 0% now maps to 10%
dimmer.setDutyCycleMax(0.9);  // 100% now maps to 90%

// Safety Limiting
dimmer.setDutyCycleLimit(0.8); // Never exceed 80% power

// Power LUT - Selectable Dimming Curve
// Choose between LINEAR or POWER LUT dimming at runtime!
//
// LINEAR MODE (default, disabled):
//   - Direct phase angle control
//   - 50% duty cycle = 50% phase delay
//   - Non-linear relationship between duty cycle and actual power output
//   - Use when you need direct phase control or working with non-resistive loads
//
// POWER LUT MODE (enabled):
//   - Non-linear curve that matches real power output of resistive loads
//   - 50% duty cycle ≈ 50% actual power consumption
//   - Also provides more natural dimming that matches human brightness perception
//   - Best for resistive loads (heating elements, incandescent bulbs) where
//     you want predictable power control
//
dimmer.enablePowerLUT(true, 10000);   // Enable Power LUT mode (semi-period: 10000us for 50Hz, 8333us for 60Hz)
dimmer.enablePowerLUT(false);         // Switch to linear phase angle mode
bool isUsing = dimmer.isPowerLUTEnabled(); // Check current mode

// Online Status Control
dimmer.setOnline(false); // Temporarily disable dimmer (e.g., when grid disconnected)
dimmer.setOnline(true);  // Re-enable dimmer

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

- **[DAC Example](https://github.com/mathieucarbou/MycilaDimmer/tree/main/examples/DAC/)** - DFRobot DAC module control
- **[JSON Example](https://github.com/mathieucarbou/MycilaDimmer/tree/main/examples/Json/)** - Telemetry and JSON integration
- **[PWM Example](https://github.com/mathieucarbou/MycilaDimmer/tree/main/examples/PWM/)** - Basic PWM dimming
- **[Thyristor Example](https://github.com/mathieucarbou/MycilaDimmer/tree/main/examples/Thyristor/)** - TRIAC control with zero-cross detection
- **[ThyristorAutoFrequency Example](https://github.com/mathieucarbou/MycilaDimmer/tree/main/examples/ThyristorAutoFrequency/)** - Automatic detection of frequency and semi-period
- **[ThyristorWithFS Example](https://github.com/mathieucarbou/MycilaDimmer/tree/main/examples/ThyristorWithFS/)** - File system integration

## Build Configuration

### PlatformIO Configuration

```ini
[platformio]
lib_dir = .
src_dir = examples/Thyristor  ; or DAC, PWM, etc.

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

## Disclaimer

_This website is provided for informational purposes only. By accessing this site and using the information contained herein, you accept the terms set forth in this disclaimer._

- _**Accuracy of Information**: We strive to provide accurate and up-to-date information on this site, but we cannot guarantee the completeness or accuracy of this information. The information provided is subject to change without notice._

- _**Use of Information**: Use of the information provided on this site is at your own risk. \*We decline all responsibility for the consequences arising from the use of this information. It is recommended that you consult a competent professional for advice specific to your situation._

- _**External Links**: This site may contain links to external websites which are provided for your reference and convenience. We have no control over the content of these external sites and we accept no responsibility for their content and their use._

- _**Limitation of Liability**: To the fullest extent permitted by applicable law, we disclaim all liability for any direct, indirect, incidental, consequential or special damages arising out of the use of, or inability to use, this website, even if we have been advised of the possibility of such damage._

_By using this site, you agree to hold harmless the owners, administrators and authors of this site from any claims arising from your use of this website. **If you do not agree to these terms, please do not use this site.**_

---

**Author**: [Mathieu Carbou](https://github.com/mathieucarbou)  
**Used in**: [YaSolR Solar Router](https://yasolr.carbou.me)
