#pragma once
#include <Arduino.h>
#include "ui_helpers.h"
#include "rotary_input.h"
#include "encoder_as5600.h"
#include "motor_control.h"

// Convert raw AS5600 (0..4095) difference into steps.
// Adjust SCALE_STEPS_PER_REV to match your mechanics.
static const uint32_t SCALE_STEPS_PER_REV = 1600; // example 200*8
static inline int16_t rawDelta(uint16_t from, uint16_t to){
  // signed shortest arc on 12-bit ring
  int16_t d = (int16_t)to - (int16_t)from;
  if (d >  2048) d -= 4096;
  if (d < -2048) d += 4096;
  return d;
}
static inline uint32_t rawToSteps(int16_t d){
  // 4096 raw ticks == 1 turn
  long s = (long)SCALE_STEPS_PER_REV * abs((long)d) / 4096L;
  return (uint32_t)s;
}

// Small prompt helper
static inline void centerTwo(const char* l1, const char* l2){
  uiBegin();
  drawRightTabTop("Back");
  drawRightTabBottom("OK");
  tft.setTextDatum(MC_DATUM); fontTitle();
  tft.drawString(l1, tft.width()/2, tft.height()/2 - 14);
  fontBody();
  tft.drawString(l2, tft.width()/2, tft.height()/2 + 12);
  tft.setTextDatum(TL_DATUM);
}

// Wait for OK or long Back (returns true if OK)
static bool waitOkOrBack(){
  while (true){
    pollInput();
    if (isSelectPressed()) return true;
    if (isBackPressedLong()) return false;
    delay(10);
  }
}

// Draw progress centered (percent)
static void progressUI(int pct){
  uiBegin();
  drawRightTabTop("Stop");
  drawRightTabBottom("Back");
  drawCenteredProgress(pct);
}

// Run continuous move to target raw until the encoder reports “close enough”
static void runToRaw(uint16_t currentRaw, uint16_t targetRaw, int speedPct){
  // crude estimate for demo: step at fixed micro delay scaled by speed
  const uint32_t baseUS = 1200; // slower base
  const uint32_t usStep = max<uint32_t>(200, (uint32_t)((100-speedPct)*baseUS/100));

  // Decide direction by shortest arc
  int16_t d = rawDelta(currentRaw, targetRaw);
  bool forward = (d>0);
  uint32_t totalSteps = rawToSteps(d);
  uint32_t done = 0;

  while (done < totalSteps){
    pollInput();
    if (isSelectPressed() || isBackPressedLong()) break;

    stepPulse();   // one step
    done++;

    int pct = (int)((done*100UL)/max<uint32_t>(1,totalSteps));
    progressUI(pct);
    delayMicroseconds(usStep);
  }
}

// ── Entry ──────────────────────────────────────────────
inline void runSingleSlideWizard(){
  // Confirm encoder present
  enc_init();

  // 1) Move to START (A)
  centerTwo("Move to START (A)","Press OK");
  if (!waitOkOrBack()) return;
  uint16_t posA = enc_getRaw();

  // 2) Move to END (B)
  centerTwo("Move to END (B)","Press OK");
  if (!waitOkOrBack()) return;
  uint16_t posB = enc_getRaw();

  // 3) Choose Mode (Time/Speed) — simple two-choice screen
  int sel = 0; // 0=Time, 1=Speed
  while (true){
    uiBegin();
    drawRightTabTop("Back");
    drawRightTabBottom("OK");

    // title
    tft.setTextDatum(TC_DATUM); fontLabel();
    tft.drawString("Choose Mode", tft.width()/2, UI::PAD + 6);
    tft.setTextDatum(TL_DATUM);

    // two rows
    int totalH = 2*UI::ITEM_H + UI::GAP;
    int yStart = (tft.height()-totalH)/2;
    drawListItemRailAware(yStart,            "Time",  sel==0);
    drawListItemRailAware(yStart+UI::ITEM_H+UI::GAP, "Speed", sel==1);

    pollInput();
    static int last = getRotaryPosition();
    int p = getRotaryPosition();
    if (p!=last){ sel += (p>last)?1:-1; last=p; sel = constrain(sel,0,1); }
    if (isSelectPressed()) break;
    if (isBackPressedLong()) return;
    delay(10);
  }

  // 4) Run
  // For now: just run from A->B at current speed setting
  uint16_t now = enc_getRaw();
  runToRaw(now, posA, getSpeedPercent());  // go to A first
  now = enc_getRaw();
  runToRaw(now, posB, getSpeedPercent());  // then to B

  // 5) Done
  uiBegin();
  drawRightTabTop("Back");
  tft.setTextDatum(MC_DATUM); fontTitle();
  tft.drawString("Done", tft.width()/2, tft.height()/2);
  tft.setTextDatum(TL_DATUM);
  while (!isSelectPressed() && !isBackPressedLong()){ pollInput(); delay(10); }
}