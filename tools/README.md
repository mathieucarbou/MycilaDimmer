# MycilaDimmer Tools

This directory contains utility scripts for the MycilaDimmer library.

## lut.py - Lookup Table Generator

Generates the firing delay lookup table used in `MycilaDimmer.cpp` for efficient TRIAC control - determining when to trigger the TRIAC to allow current flow based on desired duty cycle.

### Usage

```bash
# Generate C++ lookup table (default)
python3 tools/lut.py

# Generate with validation
python3 tools/lut.py --validate

# Generate different table sizes
python3 tools/lut.py --table-size 100

# Output formats
python3 tools/lut.py --output-format cpp     # C++ code (default)
python3 tools/lut.py --output-format csv     # CSV data

# Save to file
python3 tools/lut.py --output lut_table.h
```

### Theory

The lookup table implements the inverse of the sine square cumulative distribution function to convert power duty cycles to firing delay ratios for TRIAC-based AC dimming control.

**Phase to Duty Conversion:**
```
duty = sin(2π × phase) / (2π) - phase + 1
```

**Duty to Phase Conversion (Inverse):**
Uses bisection method to find the phase delay that produces the desired duty cycle, which determines when to fire the TRIAC.

### Table Format

- **Size**: 80 entries (configurable)
- **Range**: 16-bit values (0x0000 to 0xFFFF)
- **Mapping**: 
  - Index 0 (0% duty) → 0xFFFF (maximum firing delay - TRIAC fires very late)
  - Index 79 (100% duty) → 0x0000 (minimum firing delay - TRIAC fires immediately)
- **Interpolation**: Linear interpolation between table entries

### Validation

The script includes built-in validation that checks:
- Table monotonicity (decreasing values)
- Interpolation accuracy 
- Value range verification
- Test cases at common duty cycle points

### Example Output

```cpp
// Auto-generated lookup table for MycilaDimmer
// Table size: 80 entries
// Each entry maps duty cycle to TRIAC firing delay (16-bit values)

#define FIRING_DELAYS_LEN 80U
static constexpr uint16_t FIRING_DELAYS[FIRING_DELAYS_LEN] = {
  0xffff, 0xdfd3, 0xd734, 0xd10c, 0xcc10, 0xc7cb, 0xc401, 0xc092, 0xbd69, 0xba76,
  // ... more values ...
  0x4588, 0x4295, 0x3f6c, 0x3bfd, 0x3833, 0x33ee, 0x2ef2, 0x28ca, 0x202b, 0x0000
};
```
