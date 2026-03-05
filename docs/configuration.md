# Configuration

## Build Flags

### Thyristor Dimmer & Cycle Stealing Dimmer with ZCD

These dimmers use interrupt handlers and require IRAM-safe configuration:

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

!!! note "Semi-period is always required"
All dimmer types require `Mycila::Dimmer::setSemiPeriod()` to be called at startup (10000 for 50Hz, 8333 for 60Hz).
For `PhaseControlDimmer` subclasses (Thyristor, PWM, DFRobot), it is only strictly required when `enablePowerLUT(true)` is set — but it is good practice to always set it.
For `CycleStealingDimmer`, it is always required.

### JSON Support

Enable JSON telemetry via `toJson()`:

```ini
build_flags =
  -D MYCILA_JSON_SUPPORT

lib_deps =
  bblanchon/ArduinoJson
```

## PlatformIO Development Setup

A full development configuration for working on the library:

```ini
[platformio]
lib_dir = .
src_dir = examples/Thyristor  ; or DAC, PWM, CycleStealingWithZCD, etc.

[env]
framework = arduino
board = esp32dev
build_flags =
  -D CONFIG_ARDUHAL_LOG_COLORS
  -D CORE_DEBUG_LEVEL=ARDUHAL_LOG_LEVEL_DEBUG
  -Wall -Wextra
  ; ThyristorDimmer / CycleStealingDimmer with ZCD flags
  -D CONFIG_ARDUINO_ISR_IRAM=1
  -D CONFIG_GPTIMER_ISR_HANDLER_IN_IRAM=1
  -D CONFIG_GPTIMER_CTRL_FUNC_IN_IRAM=1
  -D CONFIG_GPTIMER_ISR_IRAM_SAFE=1
  -D CONFIG_GPIO_CTRL_FUNC_IN_IRAM=1
lib_deps =
  bblanchon/ArduinoJson
  mathieucarbou/MycilaPulseAnalyzer
```

## Troubleshooting

### Thyristor Dimmer Not Working

- Ensure the IRAM build flags listed above are set
- Check that semi-period is configured: `Mycila::Dimmer::setSemiPeriod()`
- Verify the zero-cross signal is connected and working
- Use an oscilloscope to confirm the ZCD pulse shape

### PWM Output Not Visible

- Check GPIO pin configuration
- Verify PWM frequency and resolution settings
- Use an oscilloscope or an LED to test the output signal

### DFRobot Module Not Responding

- Verify I2C wiring (SDA, SCL, power, ground)
- Check device address with an I2C scanner sketch
- Ensure the correct SKU is configured (`DFR0971`, `DFR1071`, or `DFR1073`)
