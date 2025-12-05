#ifndef ENCODER_READING_H
#define ENCODER_READING_H

#include "config.h"
#include <AS5600.h>

AS5600 as5600; // uses default I2C addr

inline void initEncoderReader() {
  Wire.begin(AS5600_SDA, AS5600_SCL);
}

inline uint16_t readRawAngle() {
  return as5600.getCumulativePosition() & 0x0FFF; // 12-bit
}

// Simple utility to convert encoder delta to mm (approx if directly on motor shaft)
inline float rawToMM(uint16_t rawDelta) {
  // revolutions = delta / 4096. then mm = rev * (pulleyTeeth * beltPitch)
  float rev = (float)rawDelta / 4096.0f;
  float mmPerRev = runtimeState.pulley_teeth * runtimeState.belt_pitch_mm;
  return rev * mmPerRev;
}

#endif
