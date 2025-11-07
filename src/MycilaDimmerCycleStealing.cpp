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

struct RegisteredDimmer;
struct RegisteredDimmer {
    Mycila::CycleStealingDimmer* dimmer = nullptr;
    RegisteredDimmer* prev = nullptr;
    RegisteredDimmer* next = nullptr;
};

static struct RegisteredDimmer* dimmers = nullptr;
static gptimer_handle_t fire_timer = nullptr;
static bool isr_running = false; // Re-entry guard: only accessed from _fireTimerISR, no volatile needed

#ifndef MYCILA_DIMMER_NO_LOCK
static portMUX_TYPE dimmers_spinlock = portMUX_INITIALIZER_UNLOCKED;
#endif

void Mycila::CycleStealingDimmer::begin() {
  if (_enabled)
    return;

  if (!GPIO_IS_VALID_OUTPUT_GPIO(_pin)) {
    ESP_LOGE(TAG, "Invalid pin: %" PRId8, _pin);
    return;
  }

  ESP_LOGI(TAG, "Enable dimmer on pin %" PRId8, _pin);

  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  _registerDimmer(this);
  _enabled = true;

  // restart with last saved value
  setDutyCycle(_dutyCycle);
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

bool Mycila::CycleStealingDimmer::_apply() {
  if (!_enabled)
    return false;
  if (!_online || !_semiPeriod || _dutyCycleFire == 0) {
    if (_running) {
      if (fire_timer != nullptr) {
        ESP_ERROR_CHECK(gptimer_set_alarm_action(fire_timer, nullptr));
      }
      _running = false;
    }
  } else {
    if (!_running) {
      // Start the firing timer with a period equal to the semi-period
      // If a ZCD is there, the start of the timer will be synced by the ZCD signal to be just before the 0V crossing
      // Otherwise, we rely on the clock accuracy considering that a ZC dimmer can only activate or deactivate at 0V crossing
      gptimer_alarm_config_t fire_timer_alarm_cfg = {
        .alarm_count = _semiPeriod,
        .reload_count = 0,
        .flags = {.auto_reload_on_alarm = true}};
      if (fire_timer != nullptr) {
        ESP_ERROR_CHECK(gptimer_set_raw_count(fire_timer, 0));
        ESP_ERROR_CHECK(gptimer_set_alarm_action(fire_timer, &fire_timer_alarm_cfg));
      }
      _running = true;
    }
  }
  return true;
}

void ARDUINO_ISR_ATTR Mycila::CycleStealingDimmer::onZeroCross(int16_t delayUntilZero, void* arg) {
  // sync the firering timer to start a little before 0V crossing
  if (inlined_gptimer_set_raw_count(fire_timer, 0) != ESP_OK) {
    // failed to reset the timer: probably not initialized yet: just ignore this ZC event
    return;
  }
}

// Timer ISR to be called as soon as a dimmer needs to be fired
bool ARDUINO_ISR_ATTR Mycila::CycleStealingDimmer::_fireTimerISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t* event, void* arg) {
  // Prevent re-entry: if this ISR takes longer than the timer period,
  // we must not allow concurrent execution which could cause race conditions
  if (isr_running) {
    // ISR is already running - skip this alarm to prevent re-entry
    return false;
  }
  isr_running = true;

  // get the time we spent looping and eventually waiting for the lock
  uint64_t fire_timer_count_value;
  if (inlined_gptimer_get_raw_count(fire_timer, &fire_timer_count_value) != ESP_OK) {
    // failed to get the timer count: just ignore this event
    isr_running = false;
    return false;
  }

  // Note: if locking takes too long (more than semi-period), a new timer event may be triggered
  // while we are still in this ISR, but it will be ignored by the isr_running guard

#ifndef MYCILA_DIMMER_NO_LOCK
  // lock since we need to iterate over the list of dimmers
  portENTER_CRITICAL_SAFE(&dimmers_spinlock);
#endif

  // go through all registered dimmers and compute the next action
  struct RegisteredDimmer* current = dimmers;
  while (current != nullptr) {
    // if (current->alarm_count != UINT16_MAX) {
    //   // this dimmer has not yet been fired (< UINT16_MAX)
    //   if (current->alarm_count <= fire_timer_count_value) {
    //     // timer alarm has reached this dimmer alarm => time to fire this dimmer
    //     gpio_ll_set_level(&GPIO, current->dimmer->_pin, HIGH);
    //     // reset the alarm count to indicate that this dimmer has been fired
    //     current->alarm_count = UINT16_MAX;
    //   } else {
    //     // dimmer has to be fired later => keep the minimum time at which we have to fire a dimmer
    //     if (current->alarm_count < fire_timer_alarm_cfg.alarm_count)
    //       fire_timer_alarm_cfg.alarm_count = current->alarm_count;
    //   }
    // }
    current = current->next;
  }

#ifndef MYCILA_DIMMER_NO_LOCK
  // unlock the list of dimmers
  portEXIT_CRITICAL_SAFE(&dimmers_spinlock);
#endif

  isr_running = false;
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
    struct RegisteredDimmer* first = new RegisteredDimmer();
    first->next = dimmers;
    dimmers->prev = first;
    dimmers = first;
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
    gptimer_stop(fire_timer); // might be already stopped
    ESP_ERROR_CHECK(gptimer_disable(fire_timer));
    ESP_ERROR_CHECK(gptimer_del_timer(fire_timer));
    fire_timer = nullptr;
  }
}
