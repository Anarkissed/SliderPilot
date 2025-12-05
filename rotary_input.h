#pragma once
#include <Arduino.h>
#include "config.h"

// Map legacy names used by various modules to your finalized pins
#define ROTARY_A_PIN   ROTARY_CLK_PIN   // KY-040 CLK
#define ROTARY_B_PIN   ROTARY_DT_PIN    // KY-040 DT
#define ROTARY_SW_PIN  BTN_OK_PIN       // press switch on encoder
#define BACK_BTN_PIN   BTN_BACK_PIN     // separate BACK button

// Polarity configuration
static const bool ROTARY_SW_ACTIVE_LOW = true;   // KY-040 switch is usually active-LOW
static const bool BACK_BTN_ACTIVE_LOW  = true;

// Debounce timings
static const uint16_t DEBOUNCE_MS      = 15;
static const uint16_t LONG_PRESS_MS    = 650;

// ----- Internal state -----
static volatile int    g_accumPos = 0;      // running position for legacy getters
static volatile int    g_stepDelta = 0;     // delta since last read
static uint8_t         g_encState = 0;      // last 2-bit state
static uint8_t         g_encHalf  = 0;      // half-step coalescer for detent=1

// Button edge/hold tracking
static bool            g_okPrev = false;
static uint32_t        g_okLastChange = 0;
static bool            g_backPrev = false;
static uint32_t        g_backLastChange = 0;
static bool            g_backLongLatched = false;

// Helpers
inline bool phys_low(uint8_t pin){ return digitalRead(pin)==LOW;  }
inline bool phys_high(uint8_t pin){ return digitalRead(pin)==HIGH; }

// Read button raw with polarity
inline bool read_ok_down(){
  bool raw = digitalRead(ROTARY_SW_PIN)==LOW;
  return ROTARY_SW_ACTIVE_LOW ? raw : !raw;
}
inline bool read_back_down(){
  bool raw = digitalRead(BACK_BTN_PIN)==LOW;
  return BACK_BTN_ACTIVE_LOW ? raw : !raw;
}

// Public API expected by the rest of the app
inline void inputInit(){
  pinMode(ROTARY_A_PIN,  INPUT_PULLUP);
  pinMode(ROTARY_B_PIN,  INPUT_PULLUP);
  pinMode(ROTARY_SW_PIN, INPUT_PULLUP);
  pinMode(BACK_BTN_PIN,  INPUT_PULLUP);

  // Initialize encoder state
  uint8_t a = digitalRead(ROTARY_A_PIN);
  uint8_t b = digitalRead(ROTARY_B_PIN);
  g_encState = ((a?1:0) << 1) | (b?1:0);

  g_accumPos = 0;
  g_stepDelta = 0;
  g_okPrev = false;
  g_backPrev = false;
  g_okLastChange = millis();
  g_backLastChange = millis();
  g_backLongLatched = false;
}

// Quadrature table: returns -1,0,+1 per transition
static inline int8_t quadStep(uint8_t prev, uint8_t curr){
  // Gray-code transitions
  // 00->01 +1, 01->11 +1, 11->10 +1, 10->00 +1
  // reverse are -1; anything else is 0
  if (prev == curr) return 0;
  switch ((prev<<2)|curr){
    case 0b0001:
    case 0b0111:
    case 0b1110:
    case 0b1000: return +1;
    case 0b0010:
    case 0b0100:
    case 0b1101:
    case 0b1011: return -1;
    default: return 0;
  }
}

// Call this every loop() to update encoder/buttons
inline void handleRotary(){
  // --- Encoder ---
  uint8_t a = digitalRead(ROTARY_A_PIN) ? 1 : 0;
  uint8_t b = digitalRead(ROTARY_B_PIN) ? 1 : 0;
  uint8_t curr = (a<<1) | b;

  int8_t step = quadStep(g_encState, curr);
  g_encState = curr;

  // Coalesce 2 half-steps into one detent (typical KY-040 behavior)
  if (step != 0){
    g_encHalf += (step>0 ? 1 : 3); // 1 forward, 3 backward mod 4
    g_encHalf &= 0x03;
    // Only update on every 2 transitions (0 -> 2 -> 0)
    // Simpler: accumulate raw transitions and only emit on even count
    static int8_t rawSum=0;
    rawSum += (step>0 ? 1 : -1);
    if (abs(rawSum) >= 2){
      int dir = (rawSum>0) ? +1 : -1;
      g_accumPos += dir;
      g_stepDelta += dir;
      rawSum = 0;
    }
  }

  // --- OK button (edge + debounce) ---
  bool nowOK = read_ok_down();
  uint32_t ms = millis();
  if (nowOK != g_okPrev){
    if ((ms - g_okLastChange) > DEBOUNCE_MS){
      g_okPrev = nowOK;
      g_okLastChange = ms;
    }
  }

  // --- BACK button (edge + debounce + long) ---
  bool nowBK = read_back_down();
  if (nowBK != g_backPrev){
    if ((ms - g_backLastChange) > DEBOUNCE_MS){
      g_backPrev = nowBK;
      g_backLastChange = ms;
      if (!nowBK){
        // released: clear long latch after release
        g_backLongLatched = false;
      }
    }
  }else{
    // held logic for long-press
    if (nowBK && !g_backLongLatched){
      if ((ms - g_backLastChange) >= LONG_PRESS_MS){
        g_backLongLatched = true; // will report true once and then latch
      }
    }
  }
}

// Read and clear delta (preferred by menus to move one per detent)
inline int getEncoderDelta(){
  int d = g_stepDelta;
  g_stepDelta = 0;
  return d;
}

// Legacy aliases used by older code
inline int getRotaryDelta(){ return getEncoderDelta(); }
inline int getRotaryPosition(){ return g_accumPos; }

// Buttons
inline bool isSelectPressed(){
  static bool prevReported=false;
  bool down = read_ok_down();
  bool edge = false;
  // positive edge (press)
  if (down && !prevReported){
    // ensure it’s stable per debounce
    if ((millis() - g_okLastChange) > DEBOUNCE_MS){
      edge = true;
      prevReported = true;
    }
  }
  if (!down) prevReported=false;
  return edge;
}

// Short back press: return true on press edge
inline bool isBackPressed(){
  static bool prevRep=false;
  bool down = read_back_down();
  bool edge=false;
  if (down && !prevRep){
    if ((millis() - g_backLastChange) > DEBOUNCE_MS){
      edge = true;
      prevRep = true;
    }
  }
  if (!down) prevRep=false;
  return edge;
}

// Long back press (some screens might still call this)
inline bool isBackPressedLong(){
  if (g_backLongLatched){
    g_backLongLatched = false; // consume
    return true;
  }
  return false;
}

// For compatibility with earlier glue layers
inline void updateRotary(){ handleRotary(); }
inline void pollInput(){ handleRotary(); }