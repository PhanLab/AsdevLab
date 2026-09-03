/*
 * Hardware Abstraction Layer (HAL) for OnStep
 * 
 * Copyright (C) 2018 Khalid Baheyeldin
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

// null decoration for non-ESP processors
#ifndef IRAM_ATTR
  #define IRAM_ATTR
#endif

#ifndef ICACHE_RAM_ATTR
  #define ICACHE_RAM_ATTR
#endif

#ifndef FPSTR
  #define FPSTR
#endif

#include "HAL_FAST_TICKS.h"

#if defined(ARDUINO_UNOWIFIR4)
  // Arduino UNO R4 WIFI
  #define MCU_STR "RENESAS RA4M1 (Arduino UNO R4 WIFI)"
  #include "esp/ESP32UnoR4WiFi.h"

#elif defined(ESP32) && ESP_ARDUINO_VERSION >= 0x30000
  // ESP32 w/libraries 3.x
  #define MCU_STR "ESP32"
  #include "esp/ESP32Libraries3.h"

#elif defined(ESP32) && ESP_ARDUINO_VERSION >= 0x20000
  // ESP32 w/libraries 2.x
  #define MCU_STR "ESP32"
  #include "esp/ESP32Libraries2.h"

#elif defined(ESP8266)
  // ESP8266
  #define MCU_STR "ESP8266"
  #include "esp/ESP8266.h"

#else
  // Generic fallback for non-ESP builds
  #warning "Unknown Platform! This HAL is configured for ESP32/ESP8266-based builds."
  #define MCU_STR "Generic (Unknown)"
  #include "default/Default.h"
#endif

#include "HAL_ANALOG.h"

#ifndef HAL_INIT
  #define HAL_INIT() do { HAL_FAST_TICKS_INIT(); } while (0)
#endif

// baseline critical task timing
#ifdef HAL_FRACTIONAL_SEC
  #define FRACTIONAL_SEC  HAL_FRACTIONAL_SEC
#else
  #define FRACTIONAL_SEC  100.0F
#endif

// progmem standin for platforms that don't have it
#ifndef CAT_ATTR
  #define CAT_ATTR
#endif

// default I2C interface
#if defined(HAL_WIRE_CLOCK)
  #define HAL_WIRE_SET_CLOCK() HAL_WIRE.setClock(HAL_WIRE_CLOCK)
#else
  #define HAL_WIRE_SET_CLOCK()
#endif
