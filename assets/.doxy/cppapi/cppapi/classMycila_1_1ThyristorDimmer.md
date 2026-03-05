

# Class Mycila::ThyristorDimmer



[**ClassList**](annotated.md) **>** [**Mycila**](namespaceMycila.md) **>** [**ThyristorDimmer**](classMycila_1_1ThyristorDimmer.md)



_Thyristor (TRIAC) based dimmer implementation for TRIAC and Random SSR dimmers._ 

* `#include <MycilaDimmerThyristor.h>`



Inherits the following classes: [Mycila::PhaseControlDimmer](classMycila_1_1PhaseControlDimmer.md)










































































## Public Functions

| Type | Name |
| ---: | :--- |
| virtual bool | [**begin**](#function-begin) () override<br>_Enable a dimmer on a specific GPIO pin._  |
| virtual void | [**end**](#function-end) () override<br>_Disable the dimmer._  |
|  uint16\_t | [**getFiringDelay**](#function-getfiringdelay) () const<br>_Get the firing delay in us of the dimmer in the range [0, semi-period] At 0% power, delay is equal to the semi-period: the dimmer is kept off At 100% power, the delay is 0 us: the dimmer is kept on This value is mostly used for TRIAC based dimmers but also in order to derive metrics based on the phase angle._  |
|  float | [**getPhaseAngle**](#function-getphaseangle) () const<br>_Get the phase angle in degrees (°) of the dimmer in the range [0, 180] At 0% power, the phase angle is equal to 180 At 100% power, the phase angle is equal to 0._  |
|  gpio\_num\_t | [**getPin**](#function-getpin) () const<br>_Get the GPIO pin used for the dimmer._  |
|  void | [**setPin**](#function-setpin) (gpio\_num\_t pin) <br>_Set the GPIO pin to use for the dimmer._  |
| virtual const char \* | [**type**](#function-type) () override const<br> |
| virtual  | [**~ThyristorDimmer**](#function-thyristordimmer) () <br> |


## Public Functions inherited from Mycila::PhaseControlDimmer

See [Mycila::PhaseControlDimmer](classMycila_1_1PhaseControlDimmer.md)

| Type | Name |
| ---: | :--- |
|  void | [**enablePowerLUT**](classMycila_1_1PhaseControlDimmer.md#function-enablepowerlut) (bool enable) <br>_Enable or disable the use of power LUT for this phase-control dimmer The power LUT provides a non-linear dimming curve that is more aligned with human perception of brightness. If disabled, a linear dimming curve will be used._  |
| virtual float | [**getPowerRatio**](classMycila_1_1PhaseControlDimmer.md#function-getpowerratio) () override const<br> |
| virtual bool | [**isOnline**](classMycila_1_1PhaseControlDimmer.md#function-isonline) () override const<br>_Returns true if the dimmer is online._  |
|  bool | [**isPowerLUTEnabled**](classMycila_1_1PhaseControlDimmer.md#function-ispowerlutenabled) () const<br>_Check if the power LUT is enabled._  |
| virtual bool | [**setDutyCycle**](classMycila_1_1PhaseControlDimmer.md#function-setdutycycle) (float dutyCycle) override<br>_Set the power duty, eventually remapped by the power LUT if enabled._  |
| virtual  | [**~PhaseControlDimmer**](classMycila_1_1PhaseControlDimmer.md#function-phasecontroldimmer) () <br> |


## Public Functions inherited from Mycila::Dimmer

See [Mycila::Dimmer](classMycila_1_1Dimmer.md)

| Type | Name |
| ---: | :--- |
| virtual bool | [**begin**](classMycila_1_1Dimmer.md#function-begin) () <br> |
|  bool | [**calculateHarmonics**](classMycila_1_1Dimmer.md#function-calculateharmonics) (float \* array, size\_t n) const<br> |
|  bool | [**calculateMetrics**](classMycila_1_1Dimmer.md#function-calculatemetrics) ([**Metrics**](structMycila_1_1Dimmer_1_1Metrics.md) & metrics, float gridVoltage, float loadResistance) const<br> |
| virtual void | [**end**](classMycila_1_1Dimmer.md#function-end) () <br> |
|  float | [**getDutyCycle**](classMycila_1_1Dimmer.md#function-getdutycycle) () const<br>_Get the power duty cycle configured for the dimmer by the user._  |
|  float | [**getDutyCycleFire**](classMycila_1_1Dimmer.md#function-getdutycyclefire) () const<br>_Get the real firing duty cycle (conduction duty cycle) applied to the dimmer in the range [0, 1]._  |
|  float | [**getDutyCycleLimit**](classMycila_1_1Dimmer.md#function-getdutycyclelimit) () const<br>_Get the power duty cycle limit of the dimmer._  |
|  float | [**getDutyCycleMapped**](classMycila_1_1Dimmer.md#function-getdutycyclemapped) () const<br>_Get the remapped power duty cycle from the currently user set duty cycle._  |
|  float | [**getDutyCycleMax**](classMycila_1_1Dimmer.md#function-getdutycyclemax) () const<br>_Get the remapped "1" of the dimmer duty cycle._  |
|  float | [**getDutyCycleMin**](classMycila_1_1Dimmer.md#function-getdutycyclemin) () const<br>_Get the remapped "0" of the dimmer duty cycle._  |
| virtual float | [**getPowerRatio**](classMycila_1_1Dimmer.md#function-getpowerratio) () const<br> |
|  bool | [**isEnabled**](classMycila_1_1Dimmer.md#function-isenabled) () const<br>_Check if the dimmer is enabled (if it was able to initialize correctly)_  |
|  bool | [**isOff**](classMycila_1_1Dimmer.md#function-isoff) () const<br>_Check if the dimmer is off._  |
|  bool | [**isOn**](classMycila_1_1Dimmer.md#function-ison) () const<br>_Check if the dimmer is on._  |
|  bool | [**isOnAtFullPower**](classMycila_1_1Dimmer.md#function-isonatfullpower) () const<br>_Check if the dimmer is on at full power._  |
| virtual bool | [**isOnline**](classMycila_1_1Dimmer.md#function-isonline) () const<br>_Returns true if the dimmer is online._  |
|  void | [**off**](classMycila_1_1Dimmer.md#function-off) () <br>_Turn off the dimmer._  |
|  void | [**on**](classMycila_1_1Dimmer.md#function-on) () <br>_Turn on the dimmer at full power._  |
| virtual bool | [**setDutyCycle**](classMycila_1_1Dimmer.md#function-setdutycycle) (float dutyCycle) <br>_Set the power duty._  |
|  void | [**setDutyCycleLimit**](classMycila_1_1Dimmer.md#function-setdutycyclelimit) (float limit) <br>_Set the power duty cycle limit of the dimmer. The duty cycle will be clamped to this limit._  |
|  void | [**setDutyCycleMax**](classMycila_1_1Dimmer.md#function-setdutycyclemax) (float max) <br>_Duty remapping (equivalent to Shelly_ [_**Dimmer**_](classMycila_1_1Dimmer.md) _remapping feature). Useful to calibrate the dimmer when using for example a PWM signal to 0-10V analog convertor connected to a voltage regulator which is only working in a specific voltage range like 1-8V._ |
|  void | [**setDutyCycleMin**](classMycila_1_1Dimmer.md#function-setdutycyclemin) (float min) <br>_Duty remapping (equivalent to Shelly_ [_**Dimmer**_](classMycila_1_1Dimmer.md) _remapping feature). Useful to calibrate the dimmer when using for example a PWM signal to 0-10V analog convertor connected to a voltage regulator which is only working in a specific voltage range like 1-8V._ |
|  void | [**setOnline**](classMycila_1_1Dimmer.md#function-setonline) (bool online) <br>_Set the online status of the dimmer._  |
| virtual const char \* | [**type**](classMycila_1_1Dimmer.md#function-type) () const<br> |
| virtual  | [**~Dimmer**](classMycila_1_1Dimmer.md#function-dimmer) () <br> |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**onZeroCross**](#function-onzerocross) (int16\_t delayUntilZero, void \* args) <br> |




## Public Static Functions inherited from Mycila::Dimmer

See [Mycila::Dimmer](classMycila_1_1Dimmer.md)

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**getSemiPeriod**](classMycila_1_1Dimmer.md#function-getsemiperiod) () <br>_Get the semi-period in us used for the power LUT calculations. If LUT is disabled, returns 0._  |
|  void | [**setSemiPeriod**](classMycila_1_1Dimmer.md#function-setsemiperiod) (uint16\_t semiPeriod) <br>_Set the semi-period of the grid frequency in us for this dimmer. This is mandatory when using power LUT._  |
















## Protected Attributes inherited from Mycila::PhaseControlDimmer

See [Mycila::PhaseControlDimmer](classMycila_1_1PhaseControlDimmer.md)

| Type | Name |
| ---: | :--- |
|  bool | [**\_powerLUTEnabled**](classMycila_1_1PhaseControlDimmer.md#variable-_powerlutenabled)   = `false`<br> |


## Protected Attributes inherited from Mycila::Dimmer

See [Mycila::Dimmer](classMycila_1_1Dimmer.md)

| Type | Name |
| ---: | :--- |
|  float | [**\_dutyCycle**](classMycila_1_1Dimmer.md#variable-_dutycycle)   = `0.0f`<br> |
|  float | [**\_dutyCycleFire**](classMycila_1_1Dimmer.md#variable-_dutycyclefire)   = `0.0f`<br> |
|  float | [**\_dutyCycleLimit**](classMycila_1_1Dimmer.md#variable-_dutycyclelimit)   = `1.0f`<br> |
|  float | [**\_dutyCycleMax**](classMycila_1_1Dimmer.md#variable-_dutycyclemax)   = `1.0f`<br> |
|  float | [**\_dutyCycleMin**](classMycila_1_1Dimmer.md#variable-_dutycyclemin)   = `0.0f`<br> |
|  bool | [**\_enabled**](classMycila_1_1Dimmer.md#variable-_enabled)   = `false`<br> |
|  bool | [**\_online**](classMycila_1_1Dimmer.md#variable-_online)   = `false`<br> |






## Protected Static Attributes inherited from Mycila::Dimmer

See [Mycila::Dimmer](classMycila_1_1Dimmer.md)

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**\_semiPeriod**](classMycila_1_1Dimmer.md#variable-_semiperiod)   = `0`<br> |






































## Protected Functions

| Type | Name |
| ---: | :--- |
| virtual bool | [**\_apply**](#function-_apply) () override<br> |


## Protected Functions inherited from Mycila::PhaseControlDimmer

See [Mycila::PhaseControlDimmer](classMycila_1_1PhaseControlDimmer.md)

| Type | Name |
| ---: | :--- |
| virtual bool | [**\_calculateDimmerHarmonics**](classMycila_1_1PhaseControlDimmer.md#function-_calculatedimmerharmonics) (float \* array, size\_t n) override const<br> |


## Protected Functions inherited from Mycila::Dimmer

See [Mycila::Dimmer](classMycila_1_1Dimmer.md)

| Type | Name |
| ---: | :--- |
| virtual bool | [**\_apply**](classMycila_1_1Dimmer.md#function-_apply) () <br> |
| virtual bool | [**\_calculateDimmerHarmonics**](classMycila_1_1Dimmer.md#function-_calculatedimmerharmonics) (float \* array, size\_t n) const<br> |




## Protected Static Functions inherited from Mycila::PhaseControlDimmer

See [Mycila::PhaseControlDimmer](classMycila_1_1PhaseControlDimmer.md)

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**\_lookupFiringDelay**](classMycila_1_1PhaseControlDimmer.md#function-_lookupfiringdelay) (float dutyCycle) <br> |


## Protected Static Functions inherited from Mycila::Dimmer

See [Mycila::Dimmer](classMycila_1_1Dimmer.md)

| Type | Name |
| ---: | :--- |
|  float | [**\_contrain**](classMycila_1_1Dimmer.md#function-_contrain) (float amt, float low, float high) <br> |


## Public Functions Documentation




### function begin 

_Enable a dimmer on a specific GPIO pin._ 
```C++
virtual bool Mycila::ThyristorDimmer::begin () override
```





**Warning:**

[**Dimmer**](classMycila_1_1Dimmer.md) won't be enabled if pin is invalid 




**Warning:**

[**Dimmer**](classMycila_1_1Dimmer.md) won't be activated until the ZCD is enabled 





        
Implements [*Mycila::Dimmer::begin*](classMycila_1_1Dimmer.md#function-begin)


<hr>



### function end 

_Disable the dimmer._ 
```C++
virtual void Mycila::ThyristorDimmer::end () override
```





**Warning:**

[**Dimmer**](classMycila_1_1Dimmer.md) won't be destroyed but won't turn on anymore even is a duty cycle is set. 





        
Implements [*Mycila::Dimmer::end*](classMycila_1_1Dimmer.md#function-end)


<hr>



### function getFiringDelay 

_Get the firing delay in us of the dimmer in the range [0, semi-period] At 0% power, delay is equal to the semi-period: the dimmer is kept off At 100% power, the delay is 0 us: the dimmer is kept on This value is mostly used for TRIAC based dimmers but also in order to derive metrics based on the phase angle._ 
```C++
inline uint16_t Mycila::ThyristorDimmer::getFiringDelay () const
```




<hr>



### function getPhaseAngle 

_Get the phase angle in degrees (°) of the dimmer in the range [0, 180] At 0% power, the phase angle is equal to 180 At 100% power, the phase angle is equal to 0._ 
```C++
inline float Mycila::ThyristorDimmer::getPhaseAngle () const
```




<hr>



### function getPin 

_Get the GPIO pin used for the dimmer._ 
```C++
inline gpio_num_t Mycila::ThyristorDimmer::getPin () const
```




<hr>



### function setPin 

_Set the GPIO pin to use for the dimmer._ 
```C++
inline void Mycila::ThyristorDimmer::setPin (
    gpio_num_t pin
) 
```




<hr>



### function type 

```C++
inline virtual const char * Mycila::ThyristorDimmer::type () override const
```



Implements [*Mycila::Dimmer::type*](classMycila_1_1Dimmer.md#function-type)


<hr>



### function ~ThyristorDimmer 

```C++
inline virtual Mycila::ThyristorDimmer::~ThyristorDimmer () 
```




<hr>
## Public Static Functions Documentation




### function onZeroCross 

```C++
static void Mycila::ThyristorDimmer::onZeroCross (
    int16_t delayUntilZero,
    void * args
) 
```



Callback to be called when a zero-crossing event is detected.



* When using MycilaPulseAnalyzer library, this callback can be registered like this:




pulseAnalyzer.onZeroCross(Mycila::ThyristorDimmer::onZeroCross);



* When using your own ISR with the RobotDyn ZCD, you can call this method with delayUntilZero == 200 since the length of the ZCD pulse is about 400 us.
* When using your own ISR with the ZCd from Daniel S, you can call this method with delayUntilZero == 550 since the length of the ZCD pulse is about 1100 us. 




        

<hr>
## Protected Functions Documentation




### function \_apply 

```C++
inline virtual bool Mycila::ThyristorDimmer::_apply () override
```



Implements [*Mycila::Dimmer::\_apply*](classMycila_1_1Dimmer.md#function-_apply)


<hr>

------------------------------
The documentation for this class was generated from the following file `src/MycilaDimmerThyristor.h`

