# Installation

## PlatformIO

Add to your `platformio.ini`:

```ini
[env:myproject]
lib_deps =
    mathieucarbou/MycilaDimmer
```

## Arduino IDE

1. Go to **Sketch** → **Include Library** → **Manage Libraries**
2. Search for "MycilaDimmer"
3. Install the library by Mathieu Carbou

## Dependencies

- **ESP32 Arduino Core** (any recent version)
- **[ArduinoJson](https://github.com/bblanchon/ArduinoJson)** (optional, for JSON telemetry support)
- **[MycilaPulseAnalyzer](https://github.com/mathieucarbou/MycilaPulseAnalyzer)** (optional, required for Thyristor and Cycle Stealing dimmers with zero-cross detection)

## Supported Platforms

- **ESP32** (all variants: ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6, ESP32-H2)
- **Arduino Framework** via PlatformIO or Arduino IDE
