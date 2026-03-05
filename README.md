# MycilaDimmer

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Continuous Integration](https://github.com/mathieucarbou/MycilaDimmer/actions/workflows/ci.yml/badge.svg)](https://github.com/mathieucarbou/MycilaDimmer/actions/workflows/ci.yml)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/mathieucarbou/library/MycilaDimmer.svg)](https://registry.platformio.org/libraries/mathieucarbou/MycilaDimmer)

A comprehensive ESP32/Arduino library for controlling AC power devices including TRIACs, SSRs, and voltage regulators through multiple dimming methods.

> **[📖 Full documentation and API reference → mathieu.carbou.me/MycilaDimmer](https://mathieu.carbou.me/MycilaDimmer/)**

![](https://mathieu.carbou.me/MycilaDimmer/assets/img/console.png)
![](https://mathieu.carbou.me/MycilaDimmer/assets/img/anim.gif)

## Overview

MycilaDimmer provides a unified interface for controlling AC power devices through different hardware implementations using a polymorphic architecture — switch between dimming methods without changing application code.

**Key Benefits:**

- **Unified API** — Same interface for all dimmer types
- **IRAM Safe** — Interrupt handlers work during flash operations
- **Hardware Agnostic** — Supports TRIACs, SSRs, and voltage regulators
- **Production Ready** — Used in [YaSolR](https://yasolr.carbou.me) Solar Router

**Dimmer Types:**

| Type                  | Use Case                                  | Method                |
| :-------------------- | :---------------------------------------- | :-------------------- |
| `ThyristorDimmer`     | TRIAC / Random SSR                        | Phase Control         |
| `CycleStealingDimmer` | Water heaters, resistive loads            | Cycle Stealing        |
| `PWMDimmer`           | PWM-to-voltage regulators (LSA, LCTC)     | Phase Control via PWM |
| `DFRobotDimmer`       | I2C DAC to voltage regulators (LSA, LCTC) | Phase Control via DAC |

## Installation

### PlatformIO

```ini
[env:myproject]
lib_deps =
  mathieucarbou/MycilaDimmer
```

### Arduino IDE

1. Go to **Sketch** → **Include Library** → **Manage Libraries**
2. Search for `MycilaDimmer`
3. Install the library by Mathieu Carbou

## Quick Start

```cpp
#include <MycilaDimmers.h>

Mycila::PWMDimmer dimmer;

void setup() {
  dimmer.setPin(GPIO_NUM_26);
  dimmer.begin();
  dimmer.setDutyCycle(0.5); // 50% power
}
```

## Configuration

### Thyristor Dimmer & Cycle Stealing Dimmer with ZCD

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

### JSON Support

```ini
build_flags =
  -D MYCILA_JSON_SUPPORT

lib_deps =
  bblanchon/ArduinoJson
```

## License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.

---

**Author**: [Mathieu Carbou](https://github.com/mathieucarbou) — **Used in**: [YaSolR Solar Router](https://yasolr.carbou.me)
