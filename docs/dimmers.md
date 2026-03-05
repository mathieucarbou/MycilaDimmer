# Dimmer Types

MycilaDimmer provides four dimmer implementations, each suited to different hardware and use cases. All share the same unified API.

## Thyristor Dimmer

Perfect for TRIAC and Random SSR control with **flicker-free progressive dimming**. Achieves smooth, continuous power control through precise phase angle control when paired with quality zero-cross detection circuits.

|                                              ESP32                                              |                                   Random Solid State Relay                                   |                                          Heat Sink                                          |                              Zero-Cross Detection Module                              |
| :---------------------------------------------------------------------------------------------: | :------------------------------------------------------------------------------------------: | :-----------------------------------------------------------------------------------------: | :-----------------------------------------------------------------------------------: |
| <img src="https://yasolr.carbou.me/assets/img/hardware/ESP32_NodeMCU.jpeg" style="width:150px"> | <img src="https://yasolr.carbou.me/assets/img/hardware/Random_SSR.jpeg" style="width:150px"> | <img src="https://yasolr.carbou.me/assets/img/hardware/Heat_Sink.jpeg" style="width:150px"> | <img src="https://yasolr.carbou.me/assets/img/hardware/ZCD.jpeg" style="width:150px"> |

```cpp
#include <MycilaDimmers.h>

Mycila::ThyristorDimmer dimmer;

void setup() {
  dimmer.setPin(GPIO_NUM_26);
  Mycila::Dimmer::setSemiPeriod(10000); // 50Hz AC (10ms semi-period) - shared across all dimmers
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

!!! tip
The Zero-Cross Detector on DIN Rail mount from Daniel S is very good and reliable and sometimes available in bulk from PCBWay. See the [YaSolR Pro page](https://yasolr.carbou.me/pro#available-hardware) for stock status.

---

## Cycle Stealing Dimmer

Perfect for resistive loads (heaters, water tank heaters) with **zero noise**. Uses an advanced First-Order Delta-Sigma Modulator (Bresenham's algorithm) to distribute power evenly across AC cycles while strictly maintaining DC balance on the grid.

|                                              ESP32                                              |                                Solid State Relay (Zero-Cross)                                |                                          Heat Sink                                          |
| :---------------------------------------------------------------------------------------------: | :------------------------------------------------------------------------------------------: | :-----------------------------------------------------------------------------------------: |
| <img src="https://yasolr.carbou.me/assets/img/hardware/ESP32_NodeMCU.jpeg" style="width:150px"> | <img src="https://yasolr.carbou.me/assets/img/hardware/SSR_40A_DA.jpeg" style="width:150px"> | <img src="https://yasolr.carbou.me/assets/img/hardware/Heat_Sink.jpeg" style="width:150px"> |

```cpp
#include <MycilaDimmers.h>

Mycila::CycleStealingDimmer dimmer;

void setup() {
  dimmer.setPin(GPIO_NUM_26);
  Mycila::Dimmer::setSemiPeriod(10000); // Required: 50Hz=10000, 60Hz=8333
  dimmer.begin();
  dimmer.setOnline(true);

  // If using a Random SSR (non zero-cross), provide ZCD signal:
  // pulseAnalyzer.onZeroCross(Mycila::CycleStealingDimmer::onZeroCross);
}
```

**Features:**

- **Zero Noise**: Switches only at zero-crossings, generating virtually no electrical noise
- **DC Balance**: Advanced algorithm ensures equal positive and negative half-cycles to prevent grid DC offset
- **High Resolution**: Delta-Sigma modulation provides precise power control at ~1% steps
- **Resistive Loads**: Ideal for water heaters, oil radiators, and other thermal loads
- **Hardware**: Works with standard Zero-Cross SSRs (cheaper) or Random SSRs with ZCD
- **No Power LUT**: `CycleStealingDimmer` does not support the Power LUT feature (it is only available on phase-control dimmers)

!!! info
See the [Cycle Stealing Algorithm](cycle_stealing.md) page for a detailed explanation of the Delta-Sigma modulator used.

---

## PWM Dimmer

Standard PWM output for PWM-to-analog converters to control voltage regulators.

|                                              ESP32                                              |                                       Voltage Regulator                                       |                                            Heat Sink                                            |                                    PWM to Analog Converter                                    |
| :---------------------------------------------------------------------------------------------: | :-------------------------------------------------------------------------------------------: | :---------------------------------------------------------------------------------------------: | :-------------------------------------------------------------------------------------------: |
| <img src="https://yasolr.carbou.me/assets/img/hardware/ESP32_NodeMCU.jpeg" style="width:150px"> | <img src="https://yasolr.carbou.me/assets/img/hardware/LSA-H3P50YB.jpeg" style="width:150px"> | <img src="https://yasolr.carbou.me/assets/img/hardware/lsa_heat_sink.jpeg" style="width:150px"> | <img src="https://yasolr.carbou.me/assets/img/hardware/PWM_33_0-10.jpeg" style="width:150px"> |

```cpp
#include <MycilaDimmers.h>

Mycila::PWMDimmer dimmer;

void setup() {
  dimmer.setPin(GPIO_NUM_26);
  dimmer.setFrequency(1000);  // 1kHz PWM
  dimmer.setResolution(12);   // 12-bit resolution
  Mycila::Dimmer::setSemiPeriod(10000); // Required when using Power LUT
  dimmer.enablePowerLUT(true);
  dimmer.begin();
  dimmer.setOnline(true);
}
```

**Features:**

- Configurable frequency (default: 1kHz)
- Configurable resolution (default: 12-bit)
- Automatic PWM channel management

For example, this dimmer can be used with a [3.3V to 0-5V/0-10V signal converter](https://aliexpress.com/item/1005004859012736.html).

---

## DFRobot DAC Dimmer

**Flicker-free progressive dimming** using precision I2C DAC modules.
Perfect for voltage regulator controlled devices ([LSA](https://aliexpress.com/item/32606780994.html), [LCTC](https://aliexpress.com/item/1005005008018888.html), etc) with 0-10V input, providing ultra-smooth dimming without any visible flickering thanks to high-resolution DAC output.

|                                              ESP32                                              |                                       Voltage Regulator                                       |                                            Heat Sink                                            |                  [DFRobot DAC](https://www.dfrobot.com/blog-13458.html)                  |
| :---------------------------------------------------------------------------------------------: | :-------------------------------------------------------------------------------------------: | :---------------------------------------------------------------------------------------------: | :--------------------------------------------------------------------------------------: |
| <img src="https://yasolr.carbou.me/assets/img/hardware/ESP32_NodeMCU.jpeg" style="width:150px"> | <img src="https://yasolr.carbou.me/assets/img/hardware/LSA-H3P50YB.jpeg" style="width:150px"> | <img src="https://yasolr.carbou.me/assets/img/hardware/lsa_heat_sink.jpeg" style="width:150px"> | <img src="https://yasolr.carbou.me/assets/img/hardware/DFR0971.jpg" style="width:150px"> |

```cpp
#include <MycilaDimmers.h>
#include <Wire.h>

Mycila::DFRobotDimmer dimmer;

void setup() {
  Wire.begin(SDA, SCL);
  dimmer.setWire(Wire);
  dimmer.setSKU(Mycila::DFRobotDimmer::SKU::DFR1071_GP8211S);
  dimmer.setDeviceAddress(0x59);
  dimmer.setOutput(Mycila::DFRobotDimmer::Output::RANGE_0_10V);
  Mycila::Dimmer::setSemiPeriod(10000); // Required when using Power LUT
  dimmer.enablePowerLUT(true);
  dimmer.begin();
  dimmer.setOnline(true);
}
```

**Supported Models:**

| Model                                                | Chip    | Resolution | Channels | Voltage Range |
| :--------------------------------------------------- | :------ | :--------- | :------- | :------------ |
| [DFR0971](https://www.dfrobot.com/product-2613.html) | GP8403  | 12-bit     | Dual     | 0-5V / 0-10V  |
| [DFR1071](https://www.dfrobot.com/product-2757.html) | GP8211S | 15-bit     | Single   | 0-5V / 0-10V  |
| [DFR1073](https://www.dfrobot.com/product-2756.html) | GP8413  | 15-bit     | Dual     | 0-5V / 0-10V  |

See the [DFRobot comparison table](https://www.dfrobot.com/blog-13458.html) for more details.
