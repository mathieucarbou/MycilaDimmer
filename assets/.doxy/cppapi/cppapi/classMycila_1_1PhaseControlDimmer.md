

# Class Mycila::PhaseControlDimmer



[**ClassList**](annotated.md) **>** [**Mycila**](namespaceMycila.md) **>** [**PhaseControlDimmer**](classMycila_1_1PhaseControlDimmer.md)





* `#include <MycilaDimmerPhaseControl.h>`



Inherits the following classes: [Mycila::Dimmer](classMycila_1_1Dimmer.md)


Inherited by the following classes: [Mycila::DFRobotDimmer](classMycila_1_1DFRobotDimmer.md),  [Mycila::PWMDimmer](classMycila_1_1PWMDimmer.md),  [Mycila::ThyristorDimmer](classMycila_1_1ThyristorDimmer.md)




















































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**enablePowerLUT**](#function-enablepowerlut) (bool enable) <br>_Enable or disable the use of power LUT for this phase-control dimmer The power LUT provides a non-linear dimming curve that is more aligned with human perception of brightness. If disabled, a linear dimming curve will be used._  |
| virtual float | [**getPowerRatio**](#function-getpowerratio) () override const<br> |
| virtual bool | [**isOnline**](#function-isonline) () override const<br>_Returns true if the dimmer is online._  |
|  bool | [**isPowerLUTEnabled**](#function-ispowerlutenabled) () const<br>_Check if the power LUT is enabled._  |
| virtual bool | [**setDutyCycle**](#function-setdutycycle) (float dutyCycle) override<br>_Set the power duty, eventually remapped by the power LUT if enabled._  |
| virtual  | [**~PhaseControlDimmer**](#function-phasecontroldimmer) () <br> |


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




## Public Static Functions inherited from Mycila::Dimmer

See [Mycila::Dimmer](classMycila_1_1Dimmer.md)

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**getSemiPeriod**](classMycila_1_1Dimmer.md#function-getsemiperiod) () <br>_Get the semi-period in us used for the power LUT calculations. If LUT is disabled, returns 0._  |
|  void | [**setSemiPeriod**](classMycila_1_1Dimmer.md#function-setsemiperiod) (uint16\_t semiPeriod) <br>_Set the semi-period of the grid frequency in us for this dimmer. This is mandatory when using power LUT._  |










## Protected Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**\_powerLUTEnabled**](#variable-_powerlutenabled)   = `false`<br> |


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
| virtual bool | [**\_calculateDimmerHarmonics**](#function-_calculatedimmerharmonics) (float \* array, size\_t n) override const<br> |


## Protected Functions inherited from Mycila::Dimmer

See [Mycila::Dimmer](classMycila_1_1Dimmer.md)

| Type | Name |
| ---: | :--- |
| virtual bool | [**\_apply**](classMycila_1_1Dimmer.md#function-_apply) () <br> |
| virtual bool | [**\_calculateDimmerHarmonics**](classMycila_1_1Dimmer.md#function-_calculatedimmerharmonics) (float \* array, size\_t n) const<br> |


## Protected Static Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**\_lookupFiringDelay**](#function-_lookupfiringdelay) (float dutyCycle) <br> |


## Protected Static Functions inherited from Mycila::Dimmer

See [Mycila::Dimmer](classMycila_1_1Dimmer.md)

| Type | Name |
| ---: | :--- |
|  float | [**\_contrain**](classMycila_1_1Dimmer.md#function-_contrain) (float amt, float low, float high) <br> |


## Public Functions Documentation




### function enablePowerLUT 

_Enable or disable the use of power LUT for this phase-control dimmer The power LUT provides a non-linear dimming curve that is more aligned with human perception of brightness. If disabled, a linear dimming curve will be used._ 
```C++
inline void Mycila::PhaseControlDimmer::enablePowerLUT (
    bool enable
) 
```





**Parameters:**


* `enable` : true to enable, false to disable 




        

<hr>



### function getPowerRatio 

```C++
inline virtual float Mycila::PhaseControlDimmer::getPowerRatio () override const
```



Implements [*Mycila::Dimmer::getPowerRatio*](classMycila_1_1Dimmer.md#function-getpowerratio)


<hr>



### function isOnline 

_Returns true if the dimmer is online._ 
```C++
inline virtual bool Mycila::PhaseControlDimmer::isOnline () override const
```



A phase-control dimmer is considered online if it is enabled, marked online, and, if power LUT is enabled, it must have a valid semi-period set. 


        
Implements [*Mycila::Dimmer::isOnline*](classMycila_1_1Dimmer.md#function-isonline)


<hr>



### function isPowerLUTEnabled 

_Check if the power LUT is enabled._ 
```C++
inline bool Mycila::PhaseControlDimmer::isPowerLUTEnabled () const
```




<hr>



### function setDutyCycle 

_Set the power duty, eventually remapped by the power LUT if enabled._ 
```C++
inline virtual bool Mycila::PhaseControlDimmer::setDutyCycle (
    float dutyCycle
) override
```





**Parameters:**


* `dutyCycle` the power duty cycle in the range [0.0, 1.0] 




        
Implements [*Mycila::Dimmer::setDutyCycle*](classMycila_1_1Dimmer.md#function-setdutycycle)


<hr>



### function ~PhaseControlDimmer 

```C++
inline virtual Mycila::PhaseControlDimmer::~PhaseControlDimmer () 
```




<hr>
## Protected Attributes Documentation




### variable \_powerLUTEnabled 

```C++
bool Mycila::PhaseControlDimmer::_powerLUTEnabled;
```




<hr>
## Protected Functions Documentation




### function \_calculateDimmerHarmonics 

```C++
inline virtual bool Mycila::PhaseControlDimmer::_calculateDimmerHarmonics (
    float * array,
    size_t n
) override const
```



Implements [*Mycila::Dimmer::\_calculateDimmerHarmonics*](classMycila_1_1Dimmer.md#function-_calculatedimmerharmonics)


<hr>
## Protected Static Functions Documentation




### function \_lookupFiringDelay 

```C++
static inline uint16_t Mycila::PhaseControlDimmer::_lookupFiringDelay (
    float dutyCycle
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/MycilaDimmerPhaseControl.h`

