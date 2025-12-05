#pragma once
#include <Wire.h>
#include <Arduino.h>

// Pins are already set in your wiring/table : SDA=GPIO21, SCL=GPIO16
#ifndef I2C_SDA_PIN
  #define I2C_SDA_PIN 21
#endif
#ifndef I2C_SCL_PIN
  #define I2C_SCL_PIN 16
#endif

// AS5600 register set
static const uint8_t AS5600_ADDR     = 0x36;
static const uint8_t AS5600_RAW_ANG_H= 0x0C;
static const uint8_t AS5600_RAW_ANG_L= 0x0D;

inline void enc_init(){
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
}

inline uint16_t enc_getRaw(){
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(AS5600_RAW_ANG_H);
  Wire.endTransmission(false);

  Wire.requestFrom((int)AS5600_ADDR, 2);
  if (Wire.available()<2) return 0;
  uint8_t hi = Wire.read();
  uint8_t lo = Wire.read();
  return ((uint16_t)hi<<8) | lo; // 0..4095 (12-bit)
}