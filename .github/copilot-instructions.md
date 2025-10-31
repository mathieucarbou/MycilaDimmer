# MycilaDimmer - AI Agent Instructions

## Project Overview

MycilaDimmer is an ESP32/Arduino library for controlling AC power devices (TRIACs, SSRs, voltage regulators) through various dimming methods. The library provides a polymorphic architecture with three concrete implementations for different hardware approaches.

## Core Architecture

### Abstract Base Class Pattern
- `Mycila::Dimmer` (src/MycilaDimmer.h) - Pure virtual base defining the contract
- All implementations inherit and implement `_apply()`, `begin()`, `end()`, and `type()`
- Virtual destructor ensures proper cleanup in polymorphic contexts

### Three Hardware Implementations
1. **ThyristorDimmer** - TRIAC control with zero-cross detection, uses ESP32 GPTimers
2. **PWMDimmer** - Standard PWM output, configurable frequency/resolution
3. **DFRobotDimmer** - I2C DAC control for DFRobot modules (GP8211S, GP8413, GP8403)

### Key Design Patterns

**Duty Cycle Mapping**: All dimmers support duty cycle remapping via `setDutyCycleMin()`/`setDutyCycleMax()` - useful for calibrating hardware that doesn't use full 0-100% range.

**Lookup Table Optimization**: Zero-cross dimmer uses a 80-entry lookup table (`TABLE_PHASE_DELAY`) with linear interpolation for efficient phase angle calculations.

**IRAM Safety**: Zero-cross implementation designed for interrupt safety with `ARDUINO_ISR_ATTR` and specific build flags for flash-concurrent operation.

## Critical Build Configuration

### Required PlatformIO Build Flags (for ThyristorDimmer)
```ini
-D CONFIG_ARDUINO_ISR_IRAM=1
-D CONFIG_GPTIMER_ISR_HANDLER_IN_IRAM=1
-D CONFIG_GPTIMER_CTRL_FUNC_IN_IRAM=1
-D CONFIG_GPTIMER_ISR_IRAM_SAFE=1
-D CONFIG_GPIO_CTRL_FUNC_IN_IRAM=1
```

### Platform-Specific Considerations
- ESP32-C3 uses GPIO_NUM_21, others use GPIO_NUM_26 (see examples)
- PWM defaults: 1kHz frequency, 12-bit resolution
- Semi-period tracking is critical for zero-cross timing

## Development Patterns

### State Management
- `_enabled` vs `isOnline()` distinction: enabled = configured, online = enabled + valid semi-period
- Duty cycle clamping with `_dutyCycleLimit` for safety
- Delay calculation: `_delay = UINT16_MAX` (off), `0` (full on), or lookup table value

### Hardware Abstraction
- GPIO pins use `gpio_num_t` type, default `GPIO_NUM_NC`
- I2C devices (DFRobot) support multiple SKUs with different resolutions
- PWM channels auto-managed, no manual channel assignment needed

### JSON Integration
Conditional compilation with `#ifdef MYCILA_JSON_SUPPORT` - include ArduinoJson dependency for telemetry features.

## Testing & Debugging

### Phase Angle Calculations
- 0% duty = 180° phase angle (full delay)
- 100% duty = 0° phase angle (no delay)
- Firing delay in microseconds: `(delay * semiPeriod) >> 16`

### Zero-Cross event Integration
Static callback `ThyristorDimmer::onZeroCross(int16_t delayUntilZero, void* args)` designed for external pulse analyzers or ISRs.

## File Organization
- Main headers: `MycilaDimmer.h` (base), `MycilaDimmer[Type].h` (implementations)
- Library config: `library.json`/`library.properties` for Arduino/PlatformIO
- Platform target: ESP32 only (`espressif32` platform)