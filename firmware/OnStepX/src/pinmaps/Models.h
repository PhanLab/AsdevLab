// -------------------------------------------------------------------------------------------------
// Loads pinmap model for current configuration
#pragma once

#if PINMAP == MaxESP3
  #define PINMAP_STR "MaxESP v3"
  #include "Pins.MaxESP3.h"
// register ASDEV_V1 ESP32 pinmap
#elif PINMAP == ASDEV_V1
  #define PINMAP_STR "ASDEV_V1"
  #include "Pins.ASDEV_V1.h"
#elif !defined(PINMAP_STR)
  #define PINMAP_STR "Unknown"
#endif

// all unassigned pins OFF
#include "Pins.defaults.h"

#include "Validate.h"