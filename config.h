#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>

// The global display object is defined in SlidePilot.ino
extern TFT_eSPI tft;

// -------------------- Pins (final mapping you’re using) --------------------
#define TMC_DIR_PIN     25
#define TMC_STEP_PIN    26
#define TMC_UART_RX     13   // TMC RX to ESP TX (GPIO13)
#define TMC_UART_TX     17   // TMC TX to ESP RX (GPIO17)

#define I2C_SDA_PIN     21   // AS5600
#define I2C_SCL_PIN     22

#define ROTARY_CLK_PIN  12   // KY-040 CLK
#define ROTARY_DT_PIN   18   // KY-040 DT
#define BTN_OK_PIN       0   // MENU/OK
#define BTN_BACK_PIN    14   // BACK

// Backlight pin (T-Display S3 LCD_BL is GPIO38)
#define BACKLIGHT_PIN   38
#define BRIGHT_LEVEL   255   // on/off for now

// Convert AS5600 angle (0..4095) to microsteps on the motor
#define STEPS_PER_ANGLE   ((float)MICROSTEPS_PER_REV / 4096.0f)

// Safe, quick reposition speed to the first point (clamped by MAX_SPEED_STEPS)
#define REHOME_SPEED_STEPS  (min((int)MAX_SPEED_STEPS, 12000))

// -------------------- Backlight control (single source of truth) ---------
inline void backlightInit() {
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);  // ON early
}
inline void backlightSet(uint8_t level) {
  digitalWrite(BACKLIGHT_PIN, (level > 0) ? HIGH : LOW);
}

// -------------------- Theme (minimal colors used elsewhere) --------------
struct Theme {
  static constexpr uint16_t BG      = TFT_BLACK;
  static constexpr uint16_t TEXT    = TFT_WHITE;
  static constexpr uint16_t TEXT_DIM= 0x8410; // gray
  static constexpr uint16_t PRIMARY = 0x7BEF; // mid gray-blue
  static constexpr uint16_t ACCENT  = 0x05FF; // cyan-ish
  static constexpr uint16_t SEP     = 0x3186; // darker gray for rails
  static constexpr uint16_t ELEV_1  = 0x2104; // subtle surface
  static constexpr uint16_t ELEV_2  = 0x1082; // deeper shadow
};

// -------------------- UI constants shared across files -------------------
namespace UI {
  static const int PAD         = 8;
  static const int ITEM_H      = 34;     // keep consistent
  static const int GAP         = 10;
  static const int RIGHT_COL_W = 44;     // rail + tabs
}

// -------------------- Small helpers available everywhere -----------------
template<typename T>
static inline T clampT(T v, T lo, T hi) { return (v < lo) ? lo : (v > hi) ? hi : v; }