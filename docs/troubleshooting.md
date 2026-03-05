# Troubleshooting

## Common Issues

### Thyristor Dimmer Not Working

- **IRAM Flags**: Ensure IRAM build flags are set in `platformio.ini`. Interrupt handlers must be in IRAM to avoid crashes during flash operations.
- **Semi-Period**: Check semi-period is configured (`Mycila::Dimmer::setSemiPeriod()`). It defaults to 0 and must be set (e.g., 10000 for 50Hz, 8333 for 60Hz).
- **Zero-Cross Signal**: Verify zero-cross signal is connected and working. You can use an oscilloscope or `MycilaPulseAnalyzer` to debug the ZCD signal.

### PWM Output Not Visible

- **GPIO Pin**: Check GPIO pin configuration matches your wiring.
- **Frequency/Resolution**: Verify PWM frequency and resolution settings are supported by the ESP32 hardware timer.
- **Testing**: Use an oscilloscope or a simple LED to visualize the PWM output.

### DFRobot Module Not Responding

- **I2C Wiring**: Verify I2C wiring (SDA, SCL, VCC, GND). ESP32 default pins vary by board.
- **Address**: Check device address with an I2C scanner sketch. Default is typically `0x58`, `0x59`, etc.
- **SKU**: Ensure correct SKU is configured (`setSKU()`) as protocols differ between GP8403, GP8211S, etc.
