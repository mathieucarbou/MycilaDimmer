// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <MycilaDimmerThyristor.h>

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

// Minimum delay to reach the voltage required for a gate current of 30mA.
// delay_us = asin((gate_resistor * gate_current) / grid_volt_max) / pi * period_us
// delay_us = asin((330 * 0.03) / 325) / pi * 10000 = 97us
#define PHASE_DELAY_MIN_US (90)

#define TAG "Thyristor"

static gptimer_handle_t fire_timer = nullptr;

#ifndef MYCILA_DIMMER_NO_LOCK
static portMUX_TYPE dimmers_spinlock = portMUX_INITIALIZER_UNLOCKED;
#endif

Mycila::ThyristorDimmer::RegisteredDimmer* Mycila::ThyristorDimmer::dimmers = nullptr;

bool Mycila::ThyristorDimmer::begin() {
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

void Mycila::ThyristorDimmer::end() {
  if (!_enabled)
    return;
  _enabled = false;
  _online = false;
  ESP_LOGI(TAG, "Disable dimmer on pin %" PRId8, _pin);
  _apply();
  _unregisterDimmer(this);
  digitalWrite(_pin, LOW);
}

void ARDUINO_ISR_ATTR Mycila::ThyristorDimmer::onZeroCross(int16_t delayUntilZero, void* arg) {
  // prepare our next alarm for the next dimmer to be fired
  gptimer_alarm_config_t fire_timer_alarm_cfg = {.alarm_count = UINT16_MAX, .reload_count = 0, .flags = {.auto_reload_on_alarm = false}};

  // immediately reset the firing timer to start counting from this ZC event and avoid it to trigger other alarms
  if (inlined_gptimer_set_raw_count(fire_timer, 0) != ESP_OK) {
    // failed to reset the timer: probably not initialized yet: just ignore this ZC event
    return;
  }

#ifndef MYCILA_DIMMER_NO_LOCK
  // lock since we need to iterate over the list of dimmers
  portENTER_CRITICAL_SAFE(&dimmers_spinlock);
#endif

  // go through all registered dimmers to prepare the next firing
  struct RegisteredDimmer* current = dimmers;
  while (current != nullptr) {
    if (current->dimmer->_delay) {
      // if a delay is applied (dimmer is off (UINT16_MAX), or on with a delay > 0), turn off the triac and it will be turned on again later
      gpio_ll_set_level(&GPIO, current->dimmer->_pin, LOW);
      // calculate the next firing time:
      // - If Dimmer is off (UINT16_MAX) => current->alarm_count == UINT16_MAX => dimmer will not be fired
      // - If Dimmer is on with a delay > 0 => check to be sure it is PHASE_DELAY_MIN_US minimum
      current->alarm_count = current->dimmer->_delay < PHASE_DELAY_MIN_US ? PHASE_DELAY_MIN_US : current->dimmer->_delay;
      // keep track the minimum delay at which we have to fire a dimmer
      if (current->alarm_count < fire_timer_alarm_cfg.alarm_count)
        fire_timer_alarm_cfg.alarm_count = current->alarm_count;
    } else {
      // no delay: dimmer has to be kept on: do nothing
      gpio_ll_set_level(&GPIO, current->dimmer->_pin, HIGH);
      // reset the alarm count to indicate that this dimmer was fired
      current->alarm_count = UINT16_MAX;
    }
    current = current->next;
  }

#ifndef MYCILA_DIMMER_NO_LOCK
  // unlock the list of dimmers
  portEXIT_CRITICAL_SAFE(&dimmers_spinlock);
#endif

  // get the time we spent looping and eventually waiting for the lock
  uint64_t fire_timer_count_value;
  if (inlined_gptimer_get_raw_count(fire_timer, &fire_timer_count_value) != ESP_OK) {
    // failed to get the timer count: just ignore this ZC event
    return;
  }

  // check if we had to wait too much time for the lock and we missed the 0V crossing point
  if (fire_timer_count_value >= delayUntilZero) {
    fire_timer_count_value -= delayUntilZero;

    // check if we missed the minimum time at which we have to turn the first dimmer on (next alarm)
    if (fire_timer_count_value <= fire_timer_alarm_cfg.alarm_count) {
      // directly call the firing ISR to turn on the first dimmer without waiting for an alarm
      if (inlined_gptimer_set_raw_count(fire_timer, fire_timer_count_value) == ESP_OK) {
        _fireTimerISR(fire_timer, nullptr, nullptr);
      }
    } else {
      // we are too late: do nothing: this is better to wait for the next ZC event than trying to turn on dimmers too late, which would create flickering
    }

  } else {
    // 0V crossing point not yet reached: set the counter to be at the right current position (very large number) before 0: the timer count will then overflow
    if (inlined_gptimer_set_raw_count(fire_timer, -static_cast<uint64_t>(delayUntilZero) + fire_timer_count_value) == ESP_OK) {
      // and set an alarm to be woken up at the right time: minimumCount
      inlined_gptimer_set_alarm_action(fire_timer, &fire_timer_alarm_cfg);
    }
  }
}

// Timer ISR to be called as soon as a dimmer needs to be fired
bool ARDUINO_ISR_ATTR Mycila::ThyristorDimmer::_fireTimerISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t* event, void* arg) {
  // prepare our next alarm for the first dimmer to be fired
  gptimer_alarm_config_t fire_timer_alarm_cfg = {.alarm_count = UINT16_MAX, .reload_count = 0, .flags = {.auto_reload_on_alarm = false}};

  // get the time we spent looping and eventually waiting for the lock
  uint64_t fire_timer_count_value;
  if (inlined_gptimer_get_raw_count(fire_timer, &fire_timer_count_value) != ESP_OK) {
    // failed to get the timer count: just ignore this event
    return false;
  }

  do {
    fire_timer_alarm_cfg.alarm_count = UINT16_MAX;

#ifndef MYCILA_DIMMER_NO_LOCK
    // lock since we need to iterate over the list of dimmers
    portENTER_CRITICAL_SAFE(&dimmers_spinlock);
#endif

    // go through all registered dimmers and check the ones to fire
    struct RegisteredDimmer* current = dimmers;
    while (current != nullptr) {
      if (current->alarm_count != UINT16_MAX) {
        // this dimmer has not yet been fired (< UINT16_MAX)
        if (current->alarm_count <= fire_timer_count_value) {
          // timer alarm has reached this dimmer alarm => time to fire this dimmer
          gpio_ll_set_level(&GPIO, current->dimmer->_pin, HIGH);
          // reset the alarm count to indicate that this dimmer has been fired
          current->alarm_count = UINT16_MAX;
        } else {
          // dimmer has to be fired later => keep the minimum time at which we have to fire a dimmer
          if (current->alarm_count < fire_timer_alarm_cfg.alarm_count)
            fire_timer_alarm_cfg.alarm_count = current->alarm_count;
        }
      }
      current = current->next;
    }

#ifndef MYCILA_DIMMER_NO_LOCK
    // unlock the list of dimmers
    portEXIT_CRITICAL_SAFE(&dimmers_spinlock);
#endif

    // refresh the current timer count value to check if we have to fire other dimmers
    inlined_gptimer_get_raw_count(fire_timer, &fire_timer_count_value);
  } while (fire_timer_alarm_cfg.alarm_count != UINT16_MAX && fire_timer_alarm_cfg.alarm_count <= fire_timer_count_value);

  // if there are some remaining dimmers to be fired, set an alarm for the next ones
  if (fire_timer_alarm_cfg.alarm_count != UINT16_MAX)
    inlined_gptimer_set_alarm_action(fire_timer, &fire_timer_alarm_cfg);

  return false;
}

// add a dimmer to the list of managed dimmers
void Mycila::ThyristorDimmer::_registerDimmer(Mycila::ThyristorDimmer* dimmer) {
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

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &fire_timer));
    gptimer_event_callbacks_t callbacks_config;
    callbacks_config.on_alarm = _fireTimerISR;
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
void Mycila::ThyristorDimmer::_unregisterDimmer(Mycila::ThyristorDimmer* dimmer) {
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
    gptimer_stop(fire_timer); // might be already stopped
    ESP_ERROR_CHECK(gptimer_disable(fire_timer));
    ESP_ERROR_CHECK(gptimer_del_timer(fire_timer));
    fire_timer = nullptr;
  }
}
