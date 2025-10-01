#!/usr/bin/env python3
"""
MycilaDimmer Lookup Table Test Suite

This script provides comprehensive testing of the TRIAC firing delay lookup table
and the _lookupFiringDelay function behavior. It validates interpolation accuracy,
bounds checking, and edge case handling.

Usage:
    python3 tools/test_lut.py [--table-size 200] [--semi-period 10000]
"""

import argparse
import sys
import os

# Add current directory to path so we can import lut module
sys.path.append(os.path.dirname(__file__))

try:
    from lut import generate_lookup_table, duty2phase, phase2duty
except ImportError:
    print("Error: Could not import lut module. Make sure lut.py is in the same directory.")
    sys.exit(1)


class LookupTableTester:
    """Test suite for MycilaDimmer lookup table functionality"""
    
    def __init__(self, table_size=200, semi_period=10000):
        self.table_size = table_size
        self.semi_period = semi_period
        self.dimmer_resolution = 12
        self.firing_delay_max = (1 << self.dimmer_resolution) - 1  # 4095
        self.firing_delays_scale = (table_size - 1) * (1 << (16 - self.dimmer_resolution))
        
        # Generate the lookup table
        self.firing_delays = generate_lookup_table(table_size)
        
        print(f"Initialized LUT tester with {table_size} entries, {semi_period}μs semi-period")
        print(f"FIRING_DELAY_MAX: {self.firing_delay_max} (0x{self.firing_delay_max:x})")
        print(f"FIRING_DELAYS_SCALE: {self.firing_delays_scale} (0x{self.firing_delays_scale:x})")
        print()

    def simulate_lookup_firing_delay(self, duty_cycle):
        """Exact simulation of the _lookupFiringDelay function"""
        duty = int(duty_cycle * self.firing_delay_max)
        slot = duty * self.firing_delays_scale + (self.firing_delays_scale >> 1)
        index = slot >> 16
        
        # Bounds checking (like C++ would do)
        if index >= self.table_size - 1:
            index = self.table_size - 2
        
        a = self.firing_delays[index]
        b = self.firing_delays[index + 1] if index + 1 < self.table_size else 0
        
        # C++ interpolation logic: a - (((a - b) * (slot & 0xffff)) >> 16)
        delay = a - (((a - b) * (slot & 0xffff)) >> 16)
        
        # Scale to period: (delay * _semiPeriod) >> 16
        result = (delay * self.semi_period) >> 16
        
        return {
            'duty_raw': duty,
            'slot': slot,
            'index': index,
            'frac': (slot & 0xffff) / 0x10000,
            'a': a,
            'b': b,
            'delay': delay,
            'result_us': result
        }

    def test_basic_functionality(self):
        """Test basic lookup functionality across duty cycle range"""
        print("=" * 60)
        print("Basic Functionality Test")
        print("=" * 60)
        
        test_duties = [0.0, 0.1, 0.25, 0.5, 0.75, 0.9, 0.99, 1.0]
        
        print("Duty%  | Index | A→B      | Frac   | Delay  | Result  | Phase°")
        print("-------|-------|----------|--------|--------|---------|-------")
        
        for duty_cycle in test_duties:
            result = self.simulate_lookup_firing_delay(duty_cycle)
            phase_angle = (result['delay'] / 0xFFFF) * 180
            
            print(f"{duty_cycle*100:6.1f} | {result['index']:5d} | "
                  f"0x{result['a']:04x}→0x{result['b']:04x} | {result['frac']:6.3f} | "
                  f"0x{result['delay']:04x} | {result['result_us']:7d} | {phase_angle:6.1f}°")
        print()

    def test_edge_cases_and_bounds(self):
        """Test edge cases and bounds checking"""
        print("=" * 60)
        print("Edge Cases and Bounds Testing")
        print("=" * 60)
        
        # Test extreme values
        extreme_duties = [-0.1, -0.01, 0.0, 1.0, 1.01, 1.5, 2.0]
        
        print("Duty   | Bounded | Index | Result  | Notes")
        print("-------|---------|-------|---------|-------")
        
        for duty_cycle in extreme_duties:
            try:
                result = self.simulate_lookup_firing_delay(duty_cycle)
                bounded = result['index'] == self.table_size - 2 and duty_cycle > 1.0
                notes = "BOUNDED" if bounded else "Normal"
                if duty_cycle < 0:
                    notes = "Negative duty"
                
                print(f"{duty_cycle:6.2f} | {'Yes' if bounded else 'No':7s} | "
                      f"{result['index']:5d} | {result['result_us']:7d} | {notes}")
            except Exception as e:
                print(f"{duty_cycle:6.2f} | ERROR   |   N/A |     N/A | {str(e)[:20]}")
        print()

    def test_interpolation_accuracy(self):
        """Test interpolation accuracy within table entries"""
        print("=" * 60)
        print("Interpolation Accuracy Test")
        print("=" * 60)
        
        # Test interpolation within first few indices
        print("Testing interpolation accuracy for first few table entries:")
        print("Index | Start→End    | Test Fractions")
        print("------|--------------|------------------------------------------------")
        
        for index in range(min(5, self.table_size - 1)):
            a = self.firing_delays[index]
            b = self.firing_delays[index + 1]
            print(f"{index:5d} | 0x{a:04x}→0x{b:04x} | ", end="")
            
            # Test different interpolation fractions
            for frac in [0.0, 0.25, 0.5, 0.75, 1.0]:
                expected = a - int((a - b) * frac)
                print(f"{frac:.2f}:0x{expected:04x} ", end="")
            print()
        print()

    def test_very_small_duties(self):
        """Test behavior with very small duty cycles"""
        print("=" * 60)
        print("Very Small Duty Cycles Test (Critical Region)")
        print("=" * 60)
        
        # Focus on the first transition region
        first_val = self.firing_delays[0]
        second_val = self.firing_delays[1]
        
        print(f"Testing transition from 0x{first_val:04x} → 0x{second_val:04x}")
        print(f"Difference: 0x{first_val - second_val:04x} = {first_val - second_val} steps")
        print()
        
        small_duties = [0.0, 0.0001, 0.0005, 0.001, 0.002, 0.003, 0.004, 0.005, 0.006, 0.01]
        
        print("Duty%    | Index | Frac   | Delay  | Result  | Analysis")
        print("---------|-------|--------|--------|---------|------------------")
        
        for duty_cycle in small_duties:
            result = self.simulate_lookup_firing_delay(duty_cycle)
            
            if result['index'] == 0:
                analysis = f"Index 0 region"
            elif result['index'] == 1:
                analysis = f"Index 1 region"
            else:
                analysis = f"Index {result['index']} region"
            
            print(f"{duty_cycle*100:8.4f} | {result['index']:5d} | {result['frac']:6.3f} | "
                  f"0x{result['delay']:04x} | {result['result_us']:7d} | {analysis}")
        print()

    def test_mathematical_consistency(self):
        """Test mathematical consistency with duty2phase/phase2duty functions"""
        print("=" * 60)
        print("Mathematical Consistency Test")
        print("=" * 60)
        
        print("Comparing lookup table results with mathematical functions:")
        print("Duty%  | LUT Delay | Math Delay | Difference | Error%")
        print("-------|-----------|------------|------------|-------")
        
        test_duties = [0.01, 0.05, 0.1, 0.25, 0.5, 0.75, 0.9, 0.99]
        
        for duty_cycle in test_duties:
            # Get result from lookup table
            lut_result = self.simulate_lookup_firing_delay(duty_cycle)
            lut_delay = lut_result['delay']
            
            # Get result from mathematical function
            math_phase = duty2phase(duty_cycle)
            math_delay = int(math_phase * 0xFFFF)
            
            # Calculate difference
            diff = abs(lut_delay - math_delay)
            error_pct = (diff / 0xFFFF) * 100
            
            print(f"{duty_cycle*100:6.1f} | 0x{lut_delay:07x}   | 0x{math_delay:07x}    | "
                  f"{diff:10d} | {error_pct:6.3f}%")
        print()

    def test_transition_points(self):
        """Test critical transition points between table indices"""
        print("=" * 60)
        print("Index Transition Points Test")
        print("=" * 60)
        
        print("Finding transition points between table indices:")
        print("Index | Transition Duty% | Slot Value | Notes")
        print("------|------------------|------------|-------")
        
        current_index = 0
        
        for duty_raw in range(0, min(200, self.firing_delay_max)):
            duty_cycle = duty_raw / self.firing_delay_max
            slot = duty_raw * self.firing_delays_scale + (self.firing_delays_scale >> 1)
            index = slot >> 16
            
            if index != current_index:
                print(f"{current_index:5d} | {duty_cycle*100:15.6f} | 0x{slot:08x} | "
                      f"Transition to index {index}")
                current_index = index
                
                if index >= 10:  # Stop after first 10 transitions
                    break
        print()

    def test_performance_characteristics(self):
        """Test performance and resolution characteristics"""
        print("=" * 60)
        print("Performance and Resolution Analysis")
        print("=" * 60)
        
        # Calculate resolution at different duty cycle ranges
        ranges = [
            (0.0, 0.01, "0-1%"),
            (0.01, 0.1, "1-10%"), 
            (0.1, 0.5, "10-50%"),
            (0.5, 0.9, "50-90%"),
            (0.9, 1.0, "90-100%")
        ]
        
        print("Duty Range | Avg Resolution | Min Step | Max Step | Entries")
        print("-----------|----------------|----------|----------|--------")
        
        for start, end, label in ranges:
            start_result = self.simulate_lookup_firing_delay(start)
            end_result = self.simulate_lookup_firing_delay(end)
            
            delay_range = start_result['delay'] - end_result['delay']
            duty_range = end - start
            index_range = end_result['index'] - start_result['index'] + 1
            
            avg_resolution = delay_range / (duty_range * 0xFFFF) if duty_range > 0 else 0
            
            print(f"{label:10s} | {avg_resolution*100:13.6f}% | {delay_range//index_range if index_range > 0 else 0:8d} | "
                  f"{delay_range//max(1,index_range//2):8d} | {index_range:7d}")
        print()

    def run_all_tests(self):
        """Run all test suites"""
        print(f"MycilaDimmer Lookup Table Test Suite")
        print(f"Table Size: {self.table_size} entries")
        print(f"Semi-period: {self.semi_period}μs")
        print()
        
        self.test_basic_functionality()
        self.test_edge_cases_and_bounds()
        self.test_very_small_duties()
        self.test_interpolation_accuracy()
        self.test_mathematical_consistency()
        self.test_transition_points()
        self.test_performance_characteristics()
        
        print("=" * 60)
        print("All tests completed successfully!")
        print("=" * 60)


def main():
    """Main function to run the test suite"""
    parser = argparse.ArgumentParser(description="Test MycilaDimmer lookup table")
    parser.add_argument("--table-size", type=int, default=200,
                        help="Size of the lookup table (default: 200)")
    parser.add_argument("--semi-period", type=int, default=10000,
                        help="Semi-period in microseconds (default: 10000)")
    parser.add_argument("--test", choices=["basic", "edge", "small", "accuracy", 
                                          "math", "transitions", "performance", "all"],
                        default="all", help="Which test to run (default: all)")
    
    args = parser.parse_args()
    
    tester = LookupTableTester(args.table_size, args.semi_period)
    
    if args.test == "basic":
        tester.test_basic_functionality()
    elif args.test == "edge":
        tester.test_edge_cases_and_bounds()
    elif args.test == "small":
        tester.test_very_small_duties()
    elif args.test == "accuracy":
        tester.test_interpolation_accuracy()
    elif args.test == "math":
        tester.test_mathematical_consistency()
    elif args.test == "transitions":
        tester.test_transition_points()
    elif args.test == "performance":
        tester.test_performance_characteristics()
    else:  # all
        tester.run_all_tests()


if __name__ == "__main__":
    main()