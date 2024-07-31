/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include "../platforms.h"

#ifdef HAL_STM32

#include "../../inc/MarlinConfig.h"

// ------------------------
// Local defines
// ------------------------

// Default timer priorities. Override by specifying alternate priorities in the board pins file.
// The TONE timer is not present here, as it currently cannot be set programmatically. It is set
// by defining TIM_IRQ_PRIO in the variant.h or platformio.ini file, which adjusts the default
// priority for STM32 HardwareTimer objects.
#define SWSERIAL_TIMER_IRQ_PRIO_DEFAULT  1 // Requires tight bit timing to communicate reliably with TMC drivers
#define SERVO_TIMER_IRQ_PRIO_DEFAULT     1 // Requires tight PWM timing to control a BLTouch reliably
#define STEP_TIMER_IRQ_PRIO_DEFAULT      2
#define TEMP_TIMER_IRQ_PRIO_DEFAULT     14 // Low priority avoids interference with other hardware and timers

#ifndef STEP_TIMER_IRQ_PRIO
  #define STEP_TIMER_IRQ_PRIO STEP_TIMER_IRQ_PRIO_DEFAULT
#endif
#ifndef TEMP_TIMER_IRQ_PRIO
  #define TEMP_TIMER_IRQ_PRIO TEMP_TIMER_IRQ_PRIO_DEFAULT
#endif
#if HAS_TMC_SW_SERIAL
  #include <SoftwareSerial.h>
  #ifndef SWSERIAL_TIMER_IRQ_PRIO
    #define SWSERIAL_TIMER_IRQ_PRIO SWSERIAL_TIMER_IRQ_PRIO_DEFAULT
  #endif
#endif
#if HAS_SERVOS
  #include "Servo.h"
  #ifndef SERVO_TIMER_IRQ_PRIO
    #define SERVO_TIMER_IRQ_PRIO SERVO_TIMER_IRQ_PRIO_DEFAULT
  #endif
#endif
#if ENABLED(SPEAKER)
  // Ensure the default timer priority is somewhere between the STEP and TEMP priorities.
  // The STM32 framework defaults to interrupt 14 for all timers. This should be increased so that
  // timing-sensitive operations such as speaker output are not impacted by the long-running
  // temperature ISR. This must be defined in the platformio.ini file or the board's variant.h,
  // so that it will be consumed by framework code.
  #if !(TIM_IRQ_PRIO > STEP_TIMER_IRQ_PRIO && TIM_IRQ_PRIO < TEMP_TIMER_IRQ_PRIO)
    #error "Default timer interrupt priority is unspecified or set to a value which may degrade performance."
  #endif
#endif

#if defined(STM32F0xx) || defined(STM32G0xx)
  #define MCU_STEP_TIMER 16
  #define MCU_TEMP_TIMER 17
#elif defined(STM32F1xx)
  #define MCU_STEP_TIMER  4
  #define MCU_TEMP_TIMER  2
#elif defined(STM32F401xC) || defined(STM32F401xE)
  #define MCU_STEP_TIMER  9           // STM32F401 has no TIM6, TIM7, or TIM8
  #define MCU_TEMP_TIMER 10
