#ifndef ENCODER_UTILS_H
#define ENCODER_UTILS_H

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

// I2C defaults (override in config.h if you already set them)
#ifndef I2C_SDA_PIN
  #define I2C_SDA_PIN 21
#endif
#ifndef I2C_SCL_PIN
  #define I2C_SCL_PIN 22
#endif
#ifndef I2C_CLOCK_HZ
  #define I2C_CLOCK_HZ 400000
#endif

// AS5600 / AMS5600 address and registers
static constexpr uint8_t AS5600_ADDR    = 0x36; // 7-bit
static constexpr uint8_t REG_RAW_ANGLE  = 0x0C; // 0x0C (MSB) / 0x0D (LSB) => 12-bit value [0..4095]
static constexpr uint8_t REG_ANGLE      = 0x0E; // filtered angle (optional)

// Internal state
static bool g_encoderPresent = false;

// Low-level I2C read 16-bit (big-endian register pair)
inline bool i2cRead16(uint8_t dev, uint8_t regMSB, uint16_t &out)
{
  Wire.beginTransmission(dev);
  Wire.write(regMSB);
  if (Wire.endTransmission(false) != 0) return false; // repeated start

  int n = Wire.requestFrom((int)dev, 2, (int)true);
  if (n != 2) return false;

  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();
  out = ((uint16_t)msb << 8) | lsb;
  return true;
}

// Initialize I2C and probe the AS5600
inline void encoderInit()
{
  static bool started = false;
  if (!started) {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_CLOCK_HZ);
    started = true;
  }

  // Probe by attempting a read
  uint16_t tmp;
  g_encoderPresent = i2cRead16(AS5600_ADDR, REG_RAW_ANGLE, tmp);
}

// Return raw 12-bit angle [0..4095]. If sensor missing, still return 0 to avoid crashes.
inline uint16_t readRawAngle()
{
  uint16_t v = 0;
  if (!g_encoderPresent) {
    // try once more in case init order changed
    encoderInit();
    if (!g_encoderPresent) return 0;
  }
  if (i2cRead16(AS5600_ADDR, REG_RAW_ANGLE, v)) {
    return (v & 0x0FFF);
  }
  // on transient error, keep UI running and return last-known-safe value 0
  return 0;
}

// Optional: filtered angle (also 12-bit). Not required, but handy for testing.
inline uint16_t readFilteredAngle()
{
  uint16_t v = 0;
  if (!g_encoderPresent) {
    encoderInit();
    if (!g_encoderPresent) return 0;
  }
  if (i2cRead16(AS5600_ADDR, REG_ANGLE, v)) {
    return (v & 0x0FFF);
  }
  return 0;
}

inline bool encoderIsPresent() { return g_encoderPresent; }

#endif // ENCODER_UTILS_H
