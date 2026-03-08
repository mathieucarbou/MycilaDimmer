

# Class Mycila::Dimmer



[**ClassList**](annotated.md) **>** [**Mycila**](namespaceMycila.md) **>** [**Dimmer**](classMycila_1_1Dimmer.md)





* `#include <MycilaDimmer.h>`





Inherited by the following classes: [Mycila::CycleStealingDimmer](classMycila_1_1CycleStealingDimmer.md),  [Mycila::PhaseControlDimmer](classMycila_1_1PhaseControlDimmer.md)










## Classes

| Type | Name |
| ---: | :--- |
| struct | [**Metrics**](structMycila_1_1Dimmer_1_1Metrics.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
| virtual bool | [**begin**](#function-begin) () <br> |
|  bool | [**calculateHarmonics**](#function-calculateharmonics) (float \* array, size\_t n) const<br> |
|  bool | [**calculateMetrics**](#function-calculatemetrics) ([**Metrics**](structMycila_1_1Dimmer_1_1Metrics.md) & metrics, float gridVoltage, float loadResistance) const<br> |
| virtual void | [**end**](#function-end) () <br> |
|  float | [**getDutyCycle**](#function-getdutycycle) () const<br>_Get the power duty cycle configured for the dimmer by the user._  |
|  float | [**getDutyCycleFire**](#function-getdutycyclefire) () const<br>_Get the real firing duty cycle (conduction duty cycle) applied to the dimmer in the range [0, 1]._  |
|  float | [**getDutyCycleLimit**](#function-getdutycyclelimit) () const<br>_Get the power duty cycle limit of the dimmer._  |
|  float | [**getDutyCycleMapped**](#function-getdutycyclemapped) () const<br>_Get the remapped power duty cycle from the currently user set duty cycle._  |
|  float | [**getDutyCycleMax**](#function-getdutycyclemax) () const<br>_Get the remapped "1" of the dimmer duty cycle._  |
|  float | [**getDutyCycleMin**](#function-getdutycyclemin) () const<br>_Get the remapped "0" of the dimmer duty cycle._  |
| virtual float | [**getPowerRatio**](#function-getpowerratio) () const<br> |
|  bool | [**isEnabled**](#function-isenabled) () const<br>_Check if the dimmer is enabled (if it was able to initialize correctly)_  |
|  bool | [**isOff**](#function-isoff) () const<br>_Check if the dimmer is off._  |
|  bool | [**isOn**](#function-ison) () const<br>_Check if the dimmer is on._  |
|  bool | [**isOnAtFullPower**](#function-isonatfullpower) () const<br>_Check if the dimmer is on at full power._  |
| virtual bool | [**isOnline**](#function-isonline) () const<br>_Returns true if the dimmer is online._  |
|  void | [**off**](#function-off) () <br>_Turn off the dimmer._  |
|  void | [**on**](#function-on) () <br>_Turn on the dimmer at full power._  |
| virtual bool | [**setDutyCycle**](#function-setdutycycle) (float dutyCycle) <br>_Set the power duty._  |
|  void | [**setDutyCycleLimit**](#function-setdutycyclelimit) (float limit) <br>_Set the power duty cycle limit of the dimmer. The duty cycle will be clamped to this limit._  |
|  void | [**setDutyCycleMax**](#function-setdutycyclemax) (float max) <br>_Duty remapping (equivalent to Shelly_ [_**Dimmer**_](classMycila_1_1Dimmer.md) _remapping feature). Useful to calibrate the dimmer when using for example a PWM signal to 0-10V analog convertor connected to a voltage regulator which is only working in a specific voltage range like 1-8V._ |
|  void | [**setDutyCycleMin**](#function-setdutycyclemin) (float min) <br>_Duty remapping (equivalent to Shelly_ [_**Dimmer**_](classMycila_1_1Dimmer.md) _remapping feature). Useful to calibrate the dimmer when using for example a PWM signal to 0-10V analog convertor connected to a voltage regulator which is only working in a specific voltage range like 1-8V._ |
|  void | [**setOnline**](#function-setonline) (bool online) <br>_Set the online status of the dimmer._  |
| virtual const char \* | [**type**](#function-type) () const<br> |
| virtual  | [**~Dimmer**](#function-dimmer) () <br> |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**getSemiPeriod**](#function-getsemiperiod) () <br>_Get the semi-period in us used for the power LUT calculations. If LUT is disabled, returns 0._  |
|  void | [**setSemiPeriod**](#function-setsemiperiod) (uint16\_t semiPeriod) <br>_Set the semi-period of the grid frequency in us for this dimmer. This is mandatory when using power LUT._  |






## Protected Attributes

| Type | Name |
| ---: | :--- |
|  float | [**\_dutyCycle**](#variable-_dutycycle)   = `0.0f`<br> |
|  float | [**\_dutyCycleFire**](#variable-_dutycyclefire)   = `0.0f`<br> |
|  float | [**\_dutyCycleLimit**](#variable-_dutycyclelimit)   = `1.0f`<br> |
|  float | [**\_dutyCycleMax**](#variable-_dutycyclemax)   = `1.0f`<br> |
|  float | [**\_dutyCycleMin**](#variable-_dutycyclemin)   = `0.0f`<br> |
|  bool | [**\_enabled**](#variable-_enabled)   = `false`<br> |
|  bool | [**\_online**](#variable-_online)   = `false`<br> |


## Protected Static Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**\_semiPeriod**](#variable-_semiperiod)   = `0`<br> |














## Protected Functions

| Type | Name |
| ---: | :--- |
| virtual bool | [**\_apply**](#function-_apply) () <br> |
| virtual bool | [**\_calculateDimmerHarmonics**](#function-_calculatedimmerharmonics) (float \* array, size\_t n) const<br> |


## Protected Static Functions

| Type | Name |
| ---: | :--- |
|  float | [**\_contrain**](#function-_contrain) (float amt, float low, float high) <br> |


## Public Functions Documentation




### function begin 

```C++
inline virtual bool Mycila::Dimmer::begin () 
```




<hr>



### function calculateHarmonics 

```C++
inline bool Mycila::Dimmer::calculateHarmonics (
    float * array,
    size_t n
) const
```




<hr>



### function calculateMetrics 

```C++
inline bool Mycila::Dimmer::calculateMetrics (
    Metrics & metrics,
    float gridVoltage,
    float loadResistance
) const
```




<hr>



### function end 

```C++
inline virtual void Mycila::Dimmer::end () 
```




<hr>



### function getDutyCycle 

_Get the power duty cycle configured for the dimmer by the user._ 
```C++
inline float Mycila::Dimmer::getDutyCycle () const
```




<hr>



### function getDutyCycleFire 

_Get the real firing duty cycle (conduction duty cycle) applied to the dimmer in the range [0, 1]._ 
```C++
inline float Mycila::Dimmer::getDutyCycleFire () const
```




* At 0% power, the ratio is equal to 0.
* At 100% power, the ratio is equal to 1. 

**Returns:**

The duty cycle applied on the hardware, or 0 if the dimmer is offline The firing ratio represents the actual proportion of time the dimmer is conducting power to the load within each AC cycle. It is computed based on the remapped duty cycle and eventually the power LUT if enabled. 







        

<hr>



### function getDutyCycleLimit 

_Get the power duty cycle limit of the dimmer._ 
```C++
inline float Mycila::Dimmer::getDutyCycleLimit () const
```




<hr>



### function getDutyCycleMapped 

_Get the remapped power duty cycle from the currently user set duty cycle._ 
```C++
inline float Mycila::Dimmer::getDutyCycleMapped () const
```




<hr>



### function getDutyCycleMax 

_Get the remapped "1" of the dimmer duty cycle._ 
```C++
inline float Mycila::Dimmer::getDutyCycleMax () const
```




<hr>



### function getDutyCycleMin 

_Get the remapped "0" of the dimmer duty cycle._ 
```C++
inline float Mycila::Dimmer::getDutyCycleMin () const
```




<hr>



### function getPowerRatio 

```C++
inline virtual float Mycila::Dimmer::getPowerRatio () const
```




<hr>



### function isEnabled 

_Check if the dimmer is enabled (if it was able to initialize correctly)_ 
```C++
inline bool Mycila::Dimmer::isEnabled () const
```




<hr>



### function isOff 

_Check if the dimmer is off._ 
```C++
inline bool Mycila::Dimmer::isOff () const
```




<hr>



### function isOn 

_Check if the dimmer is on._ 
```C++
inline bool Mycila::Dimmer::isOn () const
```




<hr>



### function isOnAtFullPower 

_Check if the dimmer is on at full power._ 
```C++
inline bool Mycila::Dimmer::isOnAtFullPower () const
```




<hr>



### function isOnline 

_Returns true if the dimmer is online._ 
```C++
inline virtual bool Mycila::Dimmer::isOnline () const
```



A dimmer is considered online if it is enabled, marked online 


        

<hr>



### function off 

_Turn off the dimmer._ 
```C++
inline void Mycila::Dimmer::off () 
```




<hr>



### function on 

_Turn on the dimmer at full power._ 
```C++
inline void Mycila::Dimmer::on () 
```




<hr>



### function setDutyCycle 

_Set the power duty._ 
```C++
inline virtual bool Mycila::Dimmer::setDutyCycle (
    float dutyCycle
) 
```





**Parameters:**


* `dutyCycle` the power duty cycle in the range [0.0, 1.0] 




        

<hr>



### function setDutyCycleLimit 

_Set the power duty cycle limit of the dimmer. The duty cycle will be clamped to this limit._ 
```C++
inline void Mycila::Dimmer::setDutyCycleLimit (
    float limit
) 
```





**Parameters:**


* `limit` the power duty cycle limit in the range [0.0, 1.0] 




        

<hr>



### function setDutyCycleMax 

_Duty remapping (equivalent to Shelly_ [_**Dimmer**_](classMycila_1_1Dimmer.md) _remapping feature). Useful to calibrate the dimmer when using for example a PWM signal to 0-10V analog convertor connected to a voltage regulator which is only working in a specific voltage range like 1-8V._
```C++
inline void Mycila::Dimmer::setDutyCycleMax (
    float max
) 
```





**Parameters:**


* `max` Set the new "1" value for the power duty cycle. The duty cycle in the range [0.0, 1.0] will be remapped to [min, max]. 




        

<hr>



### function setDutyCycleMin 

_Duty remapping (equivalent to Shelly_ [_**Dimmer**_](classMycila_1_1Dimmer.md) _remapping feature). Useful to calibrate the dimmer when using for example a PWM signal to 0-10V analog convertor connected to a voltage regulator which is only working in a specific voltage range like 1-8V._
```C++
inline void Mycila::Dimmer::setDutyCycleMin (
    float min
) 
```





**Parameters:**


* `min` Set the new "0" value for the power duty cycle. The duty cycle in the range [0.0, 1.0] will be remapped to [min, max]. 




        

<hr>



### function setOnline 

_Set the online status of the dimmer._ 
```C++
inline void Mycila::Dimmer::setOnline (
    bool online
) 
```



This flag can be used to temporarily disable the dimmer when not connected to the grid 


        

<hr>



### function type 

```C++
inline virtual const char * Mycila::Dimmer::type () const
```




<hr>



### function ~Dimmer 

```C++
inline virtual Mycila::Dimmer::~Dimmer () 
```




<hr>
## Public Static Functions Documentation




### function getSemiPeriod 

_Get the semi-period in us used for the power LUT calculations. If LUT is disabled, returns 0._ 
```C++
static inline uint16_t Mycila::Dimmer::getSemiPeriod () 
```




<hr>



### function setSemiPeriod 

_Set the semi-period of the grid frequency in us for this dimmer. This is mandatory when using power LUT._ 
```C++
static inline void Mycila::Dimmer::setSemiPeriod (
    uint16_t semiPeriod
) 
```



Typical values are 10000 for 50Hz and 8333 for 60Hz.


The value can also come from MycilaPulseAnalyzer 


        

<hr>
## Protected Attributes Documentation




### variable \_dutyCycle 

```C++
float Mycila::Dimmer::_dutyCycle;
```




<hr>



### variable \_dutyCycleFire 

```C++
float Mycila::Dimmer::_dutyCycleFire;
```




<hr>



### variable \_dutyCycleLimit 

```C++
float Mycila::Dimmer::_dutyCycleLimit;
```




<hr>



### variable \_dutyCycleMax 

```C++
float Mycila::Dimmer::_dutyCycleMax;
```




<hr>



### variable \_dutyCycleMin 

```C++
float Mycila::Dimmer::_dutyCycleMin;
```




<hr>



### variable \_enabled 

```C++
bool Mycila::Dimmer::_enabled;
```




<hr>



### variable \_online 

```C++
bool Mycila::Dimmer::_online;
```




<hr>
## Protected Static Attributes Documentation




### variable \_semiPeriod 

```C++
uint16_t Mycila::Dimmer::_semiPeriod;
```




<hr>
## Protected Functions Documentation




### function \_apply 

```C++
inline virtual bool Mycila::Dimmer::_apply () 
```




<hr>



### function \_calculateDimmerHarmonics 

```C++
inline virtual bool Mycila::Dimmer::_calculateDimmerHarmonics (
    float * array,
    size_t n
) const
```




<hr>
## Protected Static Functions Documentation




### function \_contrain 

```C++
static inline float Mycila::Dimmer::_contrain (
    float amt,
    float low,
    float high
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/MycilaDimmer.h`

