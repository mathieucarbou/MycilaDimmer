// SPDX-License-Identifier: MIT
/*
 * Copyright (C) Mathieu Carbou
 */
#include <MycilaDimmerCycleStealing.h>

// lock
#include <freertos/FreeRTOS.h>

// gpio
#include <driver/gpio.h>
#include <driver/gptimer_types.h>
#include <esp32-hal-gpio.h>
#include <hal/gpio_ll.h>
#include <soc/gpio_struct.h>

// logging
#include <esp32-hal-log.h>

// timers
#include "priv/inlined_gptimer.h"

#ifndef GPIO_IS_VALID_OUTPUT_GPIO
  #define GPIO_IS_VALID_OUTPUT_GPIO(gpio_num) ((gpio_num >= 0) && \
                                               (((1ULL << (gpio_num)) & SOC_GPIO_VALID_OUTPUT_GPIO_MASK) != 0))
#endif

#ifndef GPIO_IS_VALID_GPIO
  #define GPIO_IS_VALID_GPIO(gpio_num) ((gpio_num >= 0) && \
                                        (((1ULL << (gpio_num)) & SOC_GPIO_VALID_GPIO_MASK) != 0))
#endif

#define TAG "CycleStealing"

static gptimer_handle_t fire_timer = nullptr;
static bool inside_isr = false; // Re-entry guard: only accessed from _fireTimerISR, no volatile needed
static uint16_t alarm_set = 0;  // Remember if the alarm is set or not, and if yes, to which value

#ifndef MYCILA_DIMMER_NO_LOCK
static portMUX_TYPE dimmers_spinlock = portMUX_INITIALIZER_UNLOCKED;
#endif

Mycila::CycleStealingDimmer::RegisteredDimmer* Mycila::CycleStealingDimmer::dimmers = nullptr;

bool Mycila::CycleStealingDimmer::begin() {
  if (_enabled)
    return true;

  if (!GPIO_IS_VALID_OUTPUT_GPIO(_pin)) {
    ESP_LOGE(TAG, "Invalid pin: %" PRId8, _pin);
    return false;
  }

  ESP_LOGI(TAG, "Enable dimmer on pin %" PRId8, _pin);

  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  _registerDimmer(this);
  _enabled = true;

  // restart with last saved value
  setDutyCycle(_dutyCycle);
  return true;
}

void Mycila::CycleStealingDimmer::end() {
  if (!_enabled)
    return;
  _enabled = false;
  _online = false;
  ESP_LOGI(TAG, "Disable dimmer on pin %" PRId8, _pin);
  _apply();
  _unregisterDimmer(this);
  digitalWrite(_pin, LOW);
}

void ARDUINO_ISR_ATTR Mycila::CycleStealingDimmer::onZeroCross(int16_t delayUntilZero, void* args) {
  // read the timer count to check how much it diverted from the expected semi-period, and adjust it to stay in sync with the grid frequency
  // We allow a margin of 50us from the onZeroCross event.
  // delayUntilZero == 150us by default
  // ------ 50us -- onZeroCross --- 50us --- 100us --- 0V crossing ------
  uint64_t timer_count;
  if (inlined_gptimer_get_raw_count(fire_timer, &timer_count) != ESP_OK) {
    // failed to get the timer count: just ignore this ZC event
    return;
  }

  // should not occur, but just in case...
  if (timer_count > _semiPeriod) {
    inlined_gptimer_set_raw_count(fire_timer, _semiPeriod);
    return;
  }

  // the timer alarm was triggered less than 50us ago
  // => we allow a 50us margin of error
  if (timer_count <= 50) {
    return;
  }

  const uint16_t delayUntilAlarm = _semiPeriod - static_cast<uint16_t>(timer_count);

  // the timer alarm will trigger in less than 50us
  if (delayUntilAlarm <= 50) {
    return;
  }

  // reprogram the timer alarm
  inlined_gptimer_set_raw_count(fire_timer, _semiPeriod);
}

// Timer ISR to be called as soon as a dimmer needs to be fired
bool ARDUINO_ISR_ATTR Mycila::CycleStealingDimmer::_fireTimerISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t* event, void* arg) {
  // Prevent re-entry: if this ISR takes longer than the timer period,
  // we must not allow concurrent execution which could cause race conditions
  if (inside_isr) {
    // ISR is already running - skip this alarm to prevent re-entry
    return false;
  }
  inside_isr = true;

#ifndef MYCILA_DIMMER_NO_LOCK
  // lock since we need to iterate over the list of dimmers
  portENTER_CRITICAL_SAFE(&dimmers_spinlock);
#endif

  // Semi-period cycle stealing with balanced control
  // Each semi-period (10ms for 50Hz), we decide whether to conduct or not
  // To avoid DC components, we must balance positive and negative half-cycles

  struct RegisteredDimmer* current = dimmers;
  while (current != nullptr) {
    CycleStealingDimmer* dimmer = current->dimmer;

    // duty_milli is 0–1000 (scaled ×1000 from 0.0–1.0), pre-computed in _apply()
    // so that _fireTimerISR contains no floating-point instructions and the CPU
    // does not need to save/restore the FP coprocessor state on the ISR stack (~72 bytes).
    const uint16_t dutyCycle = current->dimmer->duty_milli;

    // Full power: always conduct
    if (dutyCycle >= 1000) {
      gpio_ll_set_level(&GPIO, dimmer->_pin, HIGH);
      current->dimmer->semi_period_odd = !current->dimmer->semi_period_odd;
      current = current->next;
      continue;
    }

    // Zero power: never conduct
    if (dutyCycle == 0) {
      gpio_ll_set_level(&GPIO, dimmer->_pin, LOW);
      current->dimmer->semi_period_odd = !current->dimmer->semi_period_odd;
      current = current->next;
      continue;
    }

    // Cycle stealing algorithm with DC balance
    // Sliding window approach (Bresenham) with polarity balancing

    // Accumulate the energy deficit
    current->dimmer->density_error += static_cast<int32_t>(dutyCycle);

    bool should_conduct = false;

    // Check if we have enough accumulated error to fire a pulse
    if (current->dimmer->density_error >= 1000) {
      // We want to fire. Check DC balance constraints.
      // semi_period_odd: True (Odd/Positive), False (Even/Negative)
      // dc_balance: 0 (Balanced), >0 (Excess Positive), <0 (Excess Negative)
      // Optimization: We define Odd as Positive (+1) and Even as Negative (-1)
      int8_t phase_val = current->dimmer->semi_period_odd ? 1 : -1;

      // Rule:
      // 1. If balanced (0), we can fire. We will create a debt.
      // 2. If unbalanced, we can ONLY fire if it reduces the imbalance (opposite sign).

      bool helps_balance = (current->dimmer->dc_balance == 0) ||
                           (current->dimmer->dc_balance > 0 && phase_val < 0) ||
                           (current->dimmer->dc_balance < 0 && phase_val > 0);

      if (helps_balance) {
        should_conduct = true;
        current->dimmer->dc_balance += phase_val;
        current->dimmer->density_error -= 1000;
      } else {
        // We need to fire for power, but it would worsen the DC imbalance.
        // Wait for the next semi-period (which will have opposite polarity).
        should_conduct = false;
      }
    }

    // Apply the decision
    gpio_ll_set_level(&GPIO, dimmer->_pin, should_conduct ? HIGH : LOW);
    current->dimmer->semi_period_odd = !current->dimmer->semi_period_odd;

    current = current->next;
  }

#ifndef MYCILA_DIMMER_NO_LOCK
  // unlock the list of dimmers
  portEXIT_CRITICAL_SAFE(&dimmers_spinlock);
#endif

  inside_isr = false;
  return false;
}

// add a dimmer to the list of managed dimmers
void Mycila::CycleStealingDimmer::_registerDimmer(Mycila::CycleStealingDimmer* dimmer) {
  if (dimmers == nullptr) {
    ESP_LOGI(TAG, "Starting dimmer firing ISR");

    gptimer_config_t timer_config;
    timer_config.clk_src = GPTIMER_CLK_SRC_DEFAULT;
    timer_config.direction = GPTIMER_COUNT_UP;
    timer_config.resolution_hz = 1000000; // 1MHz resolution
    timer_config.flags.intr_shared = true;
    timer_config.intr_priority = 0;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)
    timer_config.flags.backup_before_sleep = false;
#endif
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)
    timer_config.flags.allow_pd = false;
#endif

    gptimer_event_callbacks_t callbacks_config;
    callbacks_config.on_alarm = _fireTimerISR;

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &fire_timer));
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(fire_timer, &callbacks_config, nullptr));
    ESP_ERROR_CHECK(gptimer_enable(fire_timer));
    ESP_ERROR_CHECK(gptimer_start(fire_timer));
  }

  ESP_LOGD(TAG, "Register new dimmer %p on pin %d", dimmer, dimmer->getPin());

#ifndef MYCILA_DIMMER_NO_LOCK
  portENTER_CRITICAL_SAFE(&dimmers_spinlock);
#endif

  if (dimmers == nullptr) {
    dimmers = new RegisteredDimmer();
    dimmers->dimmer = dimmer;
  } else {
    struct RegisteredDimmer* additional = new RegisteredDimmer();
    additional->dimmer = dimmer;
    additional->next = dimmers;
    additional->prev = nullptr;
    dimmers->prev = additional;
    dimmers = additional;
  }

#ifndef MYCILA_DIMMER_NO_LOCK
  portEXIT_CRITICAL_SAFE(&dimmers_spinlock);
#endif
}

// remove a dimmer from the list of managed dimmers
void Mycila::CycleStealingDimmer::_unregisterDimmer(Mycila::CycleStealingDimmer* dimmer) {
  ESP_LOGD(TAG, "Unregister dimmer %p on pin %d", dimmer, dimmer->getPin());

#ifndef MYCILA_DIMMER_NO_LOCK
  portENTER_CRITICAL_SAFE(&dimmers_spinlock);
#endif

  struct RegisteredDimmer* current = dimmers;
  while (current != nullptr) {
    if (current->dimmer == dimmer) {
      if (current->prev != nullptr) {
        current->prev->next = current->next;
      } else {
        dimmers = current->next;
      }
      if (current->next != nullptr) {
        current->next->prev = current->prev;
      }
      delete current;
      break;
    }
    current = current->next;
  }

#ifndef MYCILA_DIMMER_NO_LOCK
  portEXIT_CRITICAL_SAFE(&dimmers_spinlock);
#endif

  if (dimmers == nullptr) {
    ESP_LOGI(TAG, "Stopping dimmer firing ISR");
    gptimer_stop(fire_timer);
    ESP_ERROR_CHECK(gptimer_disable(fire_timer));
    ESP_ERROR_CHECK(gptimer_del_timer(fire_timer));
    fire_timer = nullptr;
  }
}

bool Mycila::CycleStealingDimmer::_apply() {
  // Cache integer duty cycle for use in _fireTimerISR (avoids float arithmetic — and the
  // associated FP coprocessor context save — inside the ISR, saving ~72 bytes of ISR stack).
  duty_milli = static_cast<uint16_t>(getDutyCycleFire() * 1000.0f + 0.5f);

  if (!_enabled)
    return false;

  // we have no semi-period (or we are disconnected) => make sure the alarm is disabled to not trigger ISR
  if (_semiPeriod == 0 && alarm_set) {
    if (fire_timer != nullptr) {
      ESP_LOGD(TAG, "Disable firing timer alarm");
      ESP_ERROR_CHECK(gptimer_set_alarm_action(fire_timer, nullptr));
    }
    alarm_set = 0;

    // we have a semi-period to set and it's different from the current one => reset the alarm
  } else if (_semiPeriod > 0 && alarm_set != _semiPeriod) {
    if (fire_timer != nullptr) {
      ESP_LOGD(TAG, "Enable firing timer alarm to %" PRIu16 " us", _semiPeriod);
      gptimer_alarm_config_t fire_timer_alarm_cfg = {
        .alarm_count = _semiPeriod,
        .reload_count = 0,
        .flags = {.auto_reload_on_alarm = true}};
      ESP_ERROR_CHECK(gptimer_set_raw_count(fire_timer, 0));
      ESP_ERROR_CHECK(gptimer_set_alarm_action(fire_timer, &fire_timer_alarm_cfg));
      alarm_set = _semiPeriod;
    } else {
      alarm_set = 0;
    }
  }

  return true;
}
