// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
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

void ARDUINO_ISR_ATTR Mycila::CycleStealingDimmer::onZeroCross(int16_t delayUntilZero, void* arg) {
  // sync the firering timer to start a little before 0V crossing
  inlined_gptimer_set_raw_count(fire_timer, 0);
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

  // get the time we spent looping and eventually waiting for the lock
  uint64_t fire_timer_count_value;
  if (inlined_gptimer_get_raw_count(fire_timer, &fire_timer_count_value) != ESP_OK) {
    // failed to get the timer count: just ignore this event
    inside_isr = false;
    return false;
  }

  // Note: if locking takes too long (more than semi-period), a new timer event may be triggered
  // while we are still in this ISR, but it will be ignored by the inside_isr guard

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
    float dutyCycle = dimmer->getDutyCycleFire();

    // Full power: always conduct
    if (dutyCycle >= 1.0f) {
      gpio_ll_set_level(&GPIO, dimmer->_pin, HIGH);
      current->semi_period_odd = !current->semi_period_odd;
      current = current->next;
      continue;
    }

    // Zero power: never conduct
    if (dutyCycle <= 0.0f) {
      gpio_ll_set_level(&GPIO, dimmer->_pin, LOW);
      current->semi_period_odd = !current->semi_period_odd;
      current = current->next;
      continue;
    }

    // Cycle stealing algorithm with DC balance
    // We track the balance between odd/even semi-periods to avoid DC components

    bool should_conduct = false;

    // For balanced operation, we use a deterministic pattern based on duty cycle
    // The pattern ensures equal distribution across odd and even semi-periods

    // Calculate how many semi-periods out of a window we should conduct
    // Using a window of 20 semi-periods (200ms for 50Hz) for good resolution
    const uint8_t window_size = 20;
    uint8_t target_on_count = static_cast<uint8_t>(dutyCycle * window_size + 0.5f);

    // Increment the semi-period counter
    current->semi_period_counter++;
    if (current->semi_period_counter >= window_size) {
      current->semi_period_counter = 0;
      current->semi_period_on_count = 0;
    }

    // Balanced distribution: alternate between odd and even semi-periods
    // This prevents DC components by ensuring symmetric current draw

    if (target_on_count >= window_size / 2) {
      // High duty cycle (>= 50%): mostly on, with balanced off periods
      uint8_t target_off_count = window_size - target_on_count;
      uint8_t current_off_count = current->semi_period_counter - current->semi_period_on_count;

      // Distribute OFF periods evenly across odd and even semi-periods
      if (current_off_count < target_off_count) {
        // Need more OFF periods - check if this slot should be OFF for balance
        float off_ratio = static_cast<float>(target_off_count - current_off_count) /
                          static_cast<float>(window_size - current->semi_period_counter);
        // Alternate between odd/even to maintain balance
        should_conduct = (current->semi_period_odd && off_ratio < 0.5f) ||
                         (!current->semi_period_odd && off_ratio >= 0.5f);
      } else {
        should_conduct = true; // Already met OFF quota
      }
    } else {
      // Low duty cycle (< 50%): mostly off, with balanced on periods
      // Distribute ON periods evenly across odd and even semi-periods
      if (current->semi_period_on_count < target_on_count) {
        // Need more ON periods - check if this slot should be ON for balance
        float on_ratio = static_cast<float>(target_on_count - current->semi_period_on_count) /
                         static_cast<float>(window_size - current->semi_period_counter);
        // Alternate between odd/even to maintain balance
        should_conduct = (current->semi_period_odd && on_ratio >= 0.5f) ||
                         (!current->semi_period_odd && on_ratio < 0.5f);
      } else {
        should_conduct = false; // Already met ON quota
      }
    }

    // Apply the decision
    gpio_ll_set_level(&GPIO, dimmer->_pin, should_conduct ? HIGH : LOW);

    // Update counters
    if (should_conduct) {
      current->semi_period_on_count++;
    }
    current->semi_period_odd = !current->semi_period_odd;

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
  if (fire_timer == nullptr) {
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

  if (dimmers == nullptr && fire_timer != nullptr) {
    ESP_LOGI(TAG, "Stopping dimmer firing ISR");
    ESP_ERROR_CHECK(gptimer_set_alarm_action(fire_timer, nullptr));
    ESP_ERROR_CHECK(gptimer_stop(fire_timer));
    ESP_ERROR_CHECK(gptimer_disable(fire_timer));
    ESP_ERROR_CHECK(gptimer_del_timer(fire_timer));
    fire_timer = nullptr;
  }
}

bool Mycila::CycleStealingDimmer::_apply() {
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
