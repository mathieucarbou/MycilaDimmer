#!/usr/bin/env python3
"""MycilaDimmer Lookup Table Generator

This script generates the TRIAC firing delay lookup table used in MycilaDimmer.cpp
for efficient AC power control - determining when to trigger the TRIAC gate signal
relative to the AC zero-cross point.

The lookup table maps duty cycle (0-1) to firing delay ratio (0-1) using
the inverse sine square cumulative distribution function.

Usage:
    python lut.py [--table-size 80] [--output-format cpp|csv]
"""

import math
import argparse
from typing import List, Tuple


def phase2duty(phase: float) -> float:
    """
    TRIAC firing delay ratio [0,1] to power ratio [0,1]
    Sine square CDF (Cumulative Distribution Function)

    Args:
        phase: Firing delay ratio (0 = fire immediately, 1 = fire very late)

    Returns:
        Power ratio (0 = no power, 1 = full power)
    """
    return math.sin(2 * math.pi * phase) / (2 * math.pi) - phase + 1


def duty2phase(duty: float) -> float:
    """
    Power duty ratio [0,1] to TRIAC firing delay ratio [0,1]
    Invert Sine square CDF using Bisection method

    Args:
        duty: Power duty ratio (0 = no power, 1 = full power)

    Returns:
        Firing delay ratio (0 = fire immediately, 1 = fire very late)
    """
    if duty <= 0:
        return 1.0  # Full delay (no power)
    if duty >= 1:
        return 0.0  # No delay (full power)

    bounds = [0.0, 1.0]
    for _ in range(32):  # 32 iterations gives us ~10^-9 precision
        phase = (bounds[0] + bounds[1]) / 2
        if duty > phase2duty(phase):
            bounds[1] = phase
        else:
            bounds[0] = phase
    return (bounds[0] + bounds[1]) / 2


def generate_lookup_table(table_size: int) -> List[int]:
    """
    Generate the lookup table for duty cycle to TRIAC firing delay conversion.

    Args:
        table_size: Number of entries in the lookup table

    Returns:
        List of 16-bit integers representing TRIAC firing delays
    """
    table = []

    for i in range(table_size):
        # Map table index to duty cycle (0 to 1)
        duty = i / (table_size - 1)

        # Convert duty cycle to TRIAC firing delay ratio
        phase = duty2phase(duty)

        # Convert firing delay ratio to 16-bit integer
        # Firing delay is inverted: 0% duty = max delay, 100% duty = min delay
        delay_16bit = int(phase * 0xFFFF)

        table.append(delay_16bit)

    return table


def format_cpp_table(table: List[int], table_size: int) -> str:
    """
    Format the lookup table as C++ code.

    Args:
        table: List of 16-bit delay values
        table_size: Size of the table

    Returns:
        C++ code string
    """
    # Format as hex values, 10 per line for readability
    hex_values = [f"0x{val:04x}" for val in table]

    lines = []
    for i in range(0, len(hex_values), 10):
        line_values = hex_values[i : i + 10]
        lines.append(", ".join(line_values))

    table_content = ",\n  ".join(lines)

    return f"""// Auto-generated LUT of {table_size} entries
#define FIRING_DELAYS_LEN {table_size}U
static const uint16_t FIRING_DELAYS[FIRING_DELAYS_LEN] = {{
  {table_content}
}};"""


def format_csv_table(table: List[int]) -> str:
    """
    Format the lookup table as CSV data.

    Args:
        table: List of 16-bit delay values

    Returns:
        CSV data string
    """
    lines = ["index,duty_cycle,firing_delay_hex,firing_delay_decimal,firing_delay_ratio"]

    for i, delay in enumerate(table):
        duty = i / (len(table) - 1)
        firing_delay_ratio = delay / 0xFFFF
        lines.append(f"{i},{duty:.6f},0x{delay:04x},{delay},{firing_delay_ratio:.6f}")

    return "\n".join(lines)


def validate_table(table: List[int]) -> None:
    """
    Validate the generated lookup table.

    Args:
        table: List of 16-bit delay values
    """
    print("Table validation:")
    print(f"  Table size: {len(table)}")
    print(f"  First entry (0% duty): 0x{table[0]:04x} ({table[0]})")
    print(f"  Last entry (100% duty): 0x{table[-1]:04x} ({table[-1]})")
    print(f"  Value range: 0x{min(table):04x} - 0x{max(table):04x}")

    # Check monotonicity (table should be decreasing)
    is_monotonic = all(table[i] >= table[i + 1] for i in range(len(table) - 1))
    print(f"  Monotonic decreasing: {'✓' if is_monotonic else '✗'}")

    # Test interpolation accuracy
    test_duties = [0.1, 0.25, 0.5, 0.75, 0.9]
    print("\n  Interpolation test:")
    for duty in test_duties:
        expected_firing_delay = duty2phase(duty)
        expected_delay = int(expected_firing_delay * 0xFFFF)

        # Simulate table lookup with interpolation
        index_f = duty * (len(table) - 1)
        index = int(index_f)
        frac = index_f - index

        if index >= len(table) - 1:
            interpolated = table[-1]
        else:
            interpolated = int(table[index] * (1 - frac) + table[index + 1] * frac)

        error = abs(interpolated - expected_delay)
        error_pct = (error / 0xFFFF) * 100
        print(
            f"    Duty {duty:3.0%}: expected=0x{expected_delay:04x}, "
            f"interpolated=0x{interpolated:04x}, error={error_pct:.3f}%"
        )


def main():
    """Main function to generate and output the lookup table."""
    parser = argparse.ArgumentParser(description="Generate MycilaDimmer lookup table")
    parser.add_argument("--table-size", type=int, default=80, help="Size of the lookup table (default: 80)")
    parser.add_argument(
        "--output-format", choices=["cpp", "csv"], default="cpp", help="Output format (default: cpp)"
    )
    parser.add_argument("--validate", action="store_true", help="Validate the generated table")
    parser.add_argument("--output", "-o", type=str, help="Output file (default: stdout)")

    args = parser.parse_args()

    # Generate the lookup table
    print(f"Generating lookup table with {args.table_size} entries...")
    table = generate_lookup_table(args.table_size)

    # Validate if requested
    if args.validate:
        validate_table(table)
        print()

    # Format output
    if args.output_format == "cpp":
        output = format_cpp_table(table, args.table_size)
    elif args.output_format == "csv":
        output = format_csv_table(table)

    # Write output
    if args.output:
        with open(args.output, "w") as f:
            f.write(output)
        print(f"Table written to {args.output}")
    else:
        print(output)


if __name__ == "__main__":
    main()
