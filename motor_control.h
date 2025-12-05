#pragma once
#include <Arduino.h>
#include "config.h"   // uses clampT<> declared in your config.h

// Pins must be defined in config.h:
//   #define TMC_STEP_PIN  <pin>
//   #define TMC_DIR_PIN   <pin>
//   // #define TMC_EN_PIN <pin> (optional)

#ifndef TMC_STEP_PIN
  #error "TMC_STEP_PIN not defined. Define it in config.h (e.g. #define TMC_STEP_PIN 13)."
#endif
#ifndef TMC_DIR_PIN
  #error "TMC_DIR_PIN not defined. Define it in config.h (e.g. #define TMC_DIR_PIN 11)."
#endif

struct MotorRuntimeState {
  uint16_t current_mA    = 800;  // stored only (no UART in this minimal build)
  uint16_t microstep     = 16;   // stored only
  uint8_t  speed_percent = 50;   // 5..100
};
static MotorRuntimeState motorState;

// ---------- init & primitives ----------
inline void initMotor() {
  pinMode(TMC_STEP_PIN, OUTPUT);
  pinMode(TMC_DIR_PIN,  OUTPUT);
  digitalWrite(TMC_STEP_PIN, LOW);
  digitalWrite(TMC_DIR_PIN,  LOW);
  #ifdef TMC_EN_PIN
    pinMode(TMC_EN_PIN, OUTPUT);
    digitalWrite(TMC_EN_PIN, LOW); // enable
  #endif
}
inline void setDir(bool forward) {
  digitalWrite(TMC_DIR_PIN, forward ? HIGH : LOW);
}
inline void stepPulse() {
  digitalWrite(TMC_STEP_PIN, HIGH);
  delayMicroseconds(2);
  digitalWrite(TMC_STEP_PIN, LOW);
}

// ---------- tuning / speed ----------
inline void setMotorCurrent(uint16_t mA) {
  motorState.current_mA = clampT<uint16_t>(mA, 200, 1700);
}
inline void setMicrostepping(uint16_t ustep) {
  motorState.microstep = clampT<uint16_t>(ustep, 1, 256);
}
inline void setSpeedPercent(int pct) {
  motorState.speed_percent = clampT<int>(pct, 5, 100);
}
inline int getSpeedPercent() { return motorState.speed_percent; }

inline uint32_t usPerStepForPercent(uint8_t percent) {
  percent = clampT<uint8_t>(percent, 5, 100);
  const float minUS = 250.0f;
  const float maxUS = 4000.0f;
  float t = (100.0f - percent) / 95.0f;
  float us = minUS + (maxUS - minUS) * t * t;
  return (uint32_t)us;
}

// Blocking runs (used by wizards)
typedef bool (*CancelFunc)();

inline void runSteps(uint32_t steps, bool forward, uint8_t speedPercent = 0) {
  if (speedPercent == 0) speedPercent = motorState.speed_percent;
  setDir(forward);
  uint32_t usDelay = usPerStepForPercent(speedPercent);
  for (uint32_t i=0; i<steps; i++) {
    stepPulse();
    delayMicroseconds(usDelay);
  }
}
inline void runForMillis(bool forward, uint32_t ms, uint8_t speedPercent = 0, CancelFunc cancel = nullptr) {
  if (speedPercent == 0) speedPercent = motorState.speed_percent;
  setDir(forward);
  uint32_t usDelay = usPerStepForPercent(speedPercent);
  uint32_t start = millis();
  while ((millis() - start) < ms) {
    if (cancel && cancel()) break;
    stepPulse();
    delayMicroseconds(usDelay);
  }
}

// optional
inline void motorEnable(bool en) {
  #ifdef TMC_EN_PIN
    digitalWrite(TMC_EN_PIN, en ? LOW : HIGH);
  #endif
}
inline void checkEStopLongPress() { /* no-op stub */ }