#pragma once
#include <Arduino.h>
#include "ui_helpers.h"
#include "rotary_input.h"
#include "config.h"   // for BTN_BACK_PIN if defined

// Forward decl so the main menu can call into this
void openSettingsMenu();


// -----------------------------
// Items in Settings
// -----------------------------
static const char* const settingsItems[] = {
  "Motor Tuning",
  "Set Endpoints",
  "Motion Profile",
  "Wi-Fi Setup",
  "Units / UI"
};
static const int SETTINGS_COUNT = sizeof(settingsItems)/sizeof(settingsItems[0]);

// -----------------------------
// Layout helpers (3 visible rows, equal top/bottom margin)
// -----------------------------
static inline int rowYTopForIndex(int visibleIndex /*0..2*/) {
  const int totalVisH = 3*UI::ITEM_H + 2*UI::GAP;
  const int cy        = tft.height()/2;
  const int baseTop   = cy - totalVisH/2;
  return baseTop + visibleIndex*(UI::ITEM_H + UI::GAP);
}

// -----------------------------
// Short-press Back detector (local, debounced).
// If BTN_BACK_PIN isn't defined, it compiles away & returns false.
// -----------------------------
static inline bool backShortPressed() {
#ifdef BTN_BACK_PIN
  static uint32_t lastEdge = 0;
  static bool prev = false;
  bool now = (digitalRead(BTN_BACK_PIN) == LOW); // active low button
  bool pressed = false;

  if (now && !prev) {
    // press edge
    lastEdge = millis();
  }
  if (!now && prev) {
    // release edge: short press window
    uint32_t dur = millis() - lastEdge;
    if (dur > 40 && dur < 600) pressed = true; // debounce + short-press threshold
  }
  prev = now;
  return pressed;
#else
  return false;
#endif
}

// -----------------------------
// Draw Settings screen
// -----------------------------
inline void drawSettings() {
  uiBegin();

  // Title (subtle)
  tft.setTextDatum(TL_DATUM);
  fontLabel();
  tft.setTextColor(Theme::TEXT_DIM, Theme::BG);
  tft.setCursor(UI::PAD, 6);
  tft.print("Settings");

  // Right rail tabs
  drawRightTabTop("Back");
  drawRightTabBottom("Select");

  // Which three to show?
  extern int g_settingsIdx;
  int firstVisible = g_settingsIdx - 1;
  if (firstVisible < 0) firstVisible = 0;
  if (firstVisible > SETTINGS_COUNT-3) firstVisible = max(0, SETTINGS_COUNT-3);

  // 3 rows
  fontBody();
  for (int i = 0; i < 3; ++i) {
    int idx = firstVisible + i;
    if (idx >= SETTINGS_COUNT) break;
    int yTop = rowYTopForIndex(i);
    bool selected = (idx == g_settingsIdx);
    drawListItemRailAware(yTop, settingsItems[idx], selected);
  }

  // Slim scroll (aligned with Back/Select tabs)
  drawSlimScroll(SETTINGS_COUNT, firstVisible, 3);
}

// Keep selection index global so it persists
int g_settingsIdx = 0;

// -----------------------------
// Main loop for Settings
// -----------------------------
void openSettingsMenu() {
  // clamp index
  if (g_settingsIdx < 0) g_settingsIdx = 0;
  if (g_settingsIdx >= SETTINGS_COUNT) g_settingsIdx = SETTINGS_COUNT - 1;

  drawSettings();

  while (true) {
    updateRotary();

    // Encoder-only scrolling
    int d = getEncoderDelta();
    if (d != 0) {
      g_settingsIdx += d;
      if (g_settingsIdx < 0) g_settingsIdx = 0;
      if (g_settingsIdx >= SETTINGS_COUNT) g_settingsIdx = SETTINGS_COUNT - 1;
      drawSettings();
    }

    // SHORT PRESS BACK = EXIT (no more short-press scrolling)
    if (backShortPressed()) {
      // quick visual pulse on the "Back" tab
      drawRightTabTop("Back");
      delay(120);
      return;
    }

    // (Optional) long-press back also exits for safety
    if (isBackPressedLong()) {
      drawRightTabTop("Back");
      delay(120);
      return;
    }

    // Select pressed = pulse feedback (hook up action as needed)
    if (isSelectPressed()) {
      // Pulse the selected pill briefly
      int firstVisible = g_settingsIdx - 1;
      if (firstVisible < 0) firstVisible = 0;
      if (firstVisible > SETTINGS_COUNT-3) firstVisible = max(0, SETTINGS_COUNT-3);
      int visIdx = g_settingsIdx - firstVisible;
      if (visIdx >= 0 && visIdx < 3) {
        int yTop = rowYTopForIndex(visIdx);
        // invert pill colors momentarily
        tft.fillRoundRect(UI::PAD, yTop,
                          tft.width() - UI::RIGHT_COL_W - 2*UI::PAD,
                          UI::ITEM_H, UI::RADIUS, Theme::TEXT);
        tft.setTextColor(Theme::BG, Theme::TEXT);
        int tw = textWidth(settingsItems[g_settingsIdx]);
        int cx = UI::PAD + (tft.width() - UI::RIGHT_COL_W - 2*UI::PAD)/2;
        int cy = yTop + (UI::ITEM_H - fontHeight())/2;
        tft.setCursor(cx - tw/2, cy);
        tft.print(settingsItems[g_settingsIdx]);
        delay(110);
      }
      drawSettings();

      // TODO: route into the selected submenu here if desired
      // (e.g., if (g_settingsIdx == 0) motorTuningWizard(); etc.)
    }

    idleDimmerTick();
    delay(4);
  }
}