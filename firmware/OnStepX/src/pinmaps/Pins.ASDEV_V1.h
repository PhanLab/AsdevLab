// -------------------------------------------------------------------------------------------------
// Pin map for ASDEV_V1
//
// MCU: ESP32 DevKit V1 / ESP-WROOM-32, 38 pin
//
// Axis1: TMC2209 - ALT / RA
// Axis2: TMC2209 - AZ / DEC
// Axis3: A4988   - Rotator
// Axis4: A4988   - Focuser
//
// Home Axis1: GPIO34
// Home Axis2: GPIO35
//
// Flip Mirror Servo: GPIO18
// Status LED: GPIO19
//
// ESP32 <-> ESP32-P4 UART:
// ESP32 GPIO22 TX -> ESP32-P4 GPIO2 RX
// ESP32 GPIO21 RX <- ESP32-P4 GPIO1 TX
//
// TMC2209 UART:
// ESP32 GPIO23 TX
// ESP32 GPIO39 RX
// Baud: 460800
//
// -------------------------------------------------------------------------------------------------

#pragma once

#if defined(ESP32)

// -------------------------------------------------------------------------------------------------
// SERIAL PORTS
// -------------------------------------------------------------------------------------------------

// Serial0:
// RX = GPIO3
// TX = GPIO1
// Usually USB programming / debug serial

#if SERIAL_A_BAUD_DEFAULT != OFF
  #define SERIAL_A Serial
#endif

// Serial2 is used for communication with ESP32-P4
//
// ESP32 GPIO22 TX -> P4 GPIO2 RX
// ESP32 GPIO21 RX <- P4 GPIO1 TX

#if SERIAL_B_BAUD_DEFAULT != OFF
  #define SERIAL_B Serial2
  #define SERIAL_B_RX 21
  #define SERIAL_B_TX 22
#endif


// -------------------------------------------------------------------------------------------------
// TMC2209 HARDWARE UART
// -------------------------------------------------------------------------------------------------

#if defined(STEP_DIR_TMC_UART_PRESENT) || defined(SERVO_TMC2209_PRESENT)

  #define SERIAL_TMC_HARDWARE_UART

  // Use ESP32 hardware UART1 for TMC2209 communication
  #define SERIAL_TMC Serial1

  // TMC2209 UART baud rate
  #define SERIAL_TMC_BAUD 460800

  // ESP32 UART1 pins
  //
  // GPIO23 = TX
  // GPIO39 = RX
  //
  // GPIO39 is input-only, therefore suitable for UART RX.
  #define SERIAL_TMC_RX 39
  #define SERIAL_TMC_TX 23

  // TMC UART addresses:
  //
  // Axis1 = address 0
  // Axis2 = address 1
  //
  // Axis3 and Axis4 are A4988 and do not use TMC UART.
  #define SERIAL_TMC_ADDRESS_MAP(x) ((x) == 0 ? 0 : \
                                     (x) == 1 ? 1 : 0)

#endif


// -------------------------------------------------------------------------------------------------
// AUXILIARY PINS
// -------------------------------------------------------------------------------------------------

// Home sensors
#define AUX3_PIN 34
#define AUX4_PIN 35

// Flip mirror servo signal
#define AUX5_PIN 18

// Status LED
#define AUX8_PIN 19


// -------------------------------------------------------------------------------------------------
// HOME SENSORS
// -------------------------------------------------------------------------------------------------

#ifndef AXIS1_SENSE_HOME_PIN
  #define AXIS1_SENSE_HOME_PIN AUX3_PIN
#endif

#ifndef AXIS2_SENSE_HOME_PIN
  #define AXIS2_SENSE_HOME_PIN AUX4_PIN
#endif


// -------------------------------------------------------------------------------------------------
// STATUS LED
// -------------------------------------------------------------------------------------------------

#ifndef STATUS_LED_PIN
  #define STATUS_LED_PIN AUX8_PIN
#endif

#ifndef MOUNT_LED_PIN
  #define MOUNT_LED_PIN STATUS_LED_PIN
#endif


// -------------------------------------------------------------------------------------------------
// FLIP MIRROR SERVO
//
// GPIO18 is reserved for the flip mirror servo.
//
// The exact feature mapping for this servo should be configured separately
// in the OnStepX feature/servo configuration.
// -------------------------------------------------------------------------------------------------

#define FLIP_MIRROR_SERVO_PIN AUX5_PIN


// -------------------------------------------------------------------------------------------------
// SHARED ENABLE
//
// GPIO13 controls EN on:
//
// TMC2209 Axis1
// TMC2209 Axis2
// A4988 Axis3
// A4988 Axis4
// -------------------------------------------------------------------------------------------------

#define SHARED_ENABLE_PIN 13


// -------------------------------------------------------------------------------------------------
// AXIS 1
//
// TMC2209
// ALT / RA
//
// STEP = GPIO27
// DIR  = GPIO26
// EN   = GPIO13 (shared)
// -------------------------------------------------------------------------------------------------

#define AXIS1_ENABLE_PIN SHARED

#define AXIS1_M0_PIN OFF
#define AXIS1_M1_PIN OFF
#define AXIS1_M2_PIN OFF
#define AXIS1_M3_PIN OFF

#define AXIS1_STEP_PIN 27
#define AXIS1_DIR_PIN 26


// -------------------------------------------------------------------------------------------------
// AXIS 2
//
// TMC2209
// AZ / DEC
//
// STEP = GPIO33
// DIR  = GPIO32
// EN   = GPIO13 (shared)
// -------------------------------------------------------------------------------------------------

#define AXIS2_ENABLE_PIN SHARED

#define AXIS2_M0_PIN OFF
#define AXIS2_M1_PIN OFF
#define AXIS2_M2_PIN OFF
#define AXIS2_M3_PIN OFF

#define AXIS2_STEP_PIN 33
#define AXIS2_DIR_PIN 32


// -------------------------------------------------------------------------------------------------
// AXIS 3
//
// A4988
// ROTATOR
//
// STEP = GPIO14
// DIR  = GPIO25
// EN   = GPIO13 (shared)
//
// MS1/MS2/MS3 of A4988 are hardware configured.
// -------------------------------------------------------------------------------------------------

#define AXIS3_ENABLE_PIN SHARED

#define AXIS3_M0_PIN OFF
#define AXIS3_M1_PIN OFF
#define AXIS3_M2_PIN OFF
#define AXIS3_M3_PIN OFF

#define AXIS3_STEP_PIN 14
#define AXIS3_DIR_PIN 25


// -------------------------------------------------------------------------------------------------
// AXIS 4
//
// A4988
// FOCUSER
//
// STEP = GPIO4
// DIR  = GPIO16
// EN   = GPIO13 (shared)
//
// MS1/MS2/MS3 of A4988 are hardware configured.
// -------------------------------------------------------------------------------------------------

#define AXIS4_ENABLE_PIN SHARED

#define AXIS4_M0_PIN OFF
#define AXIS4_M1_PIN OFF
#define AXIS4_M2_PIN OFF
#define AXIS4_M3_PIN OFF

#define AXIS4_STEP_PIN 4
#define AXIS4_DIR_PIN 16


// -------------------------------------------------------------------------------------------------
// UNUSED AXES
// -------------------------------------------------------------------------------------------------

#define AXIS5_ENABLE_PIN OFF
#define AXIS5_STEP_PIN OFF
#define AXIS5_DIR_PIN OFF

#define AXIS6_ENABLE_PIN OFF
#define AXIS6_STEP_PIN OFF
#define AXIS6_DIR_PIN OFF

#define AXIS7_ENABLE_PIN OFF
#define AXIS7_STEP_PIN OFF
#define AXIS7_DIR_PIN OFF

#define AXIS8_ENABLE_PIN OFF
#define AXIS8_STEP_PIN OFF
#define AXIS8_DIR_PIN OFF

#define AXIS9_ENABLE_PIN OFF
#define AXIS9_STEP_PIN OFF
#define AXIS9_DIR_PIN OFF


// -------------------------------------------------------------------------------------------------
// ST4
// -------------------------------------------------------------------------------------------------

#define ST4_RA_W_PIN OFF
#define ST4_DEC_S_PIN OFF
#define ST4_DEC_N_PIN OFF
#define ST4_RA_E_PIN OFF


#else

  #error "ASDEV_V1 requires an ESP32 processor!"

#endif