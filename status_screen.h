#ifndef STATUS_SCREEN_H
#define STATUS_SCREEN_H

#include "ui_helpers.h"
#include "rotary_input.h"
#include "wizard_ui.h"
#include "config.h"

inline void openStatusScreen() {
  uint32_t start = millis();
  while (true) {
    wizardFrameStart("OK");

    tft.setTextDatum(TL_DATUM); fontLabel(); tft.setTextColor(Theme::TEXT, Theme::BG);
    int x = UI::PAD + 6; int y = 20;
    tft.setCursor(x,y); tft.print("Status"); y += fontHeight() + 6;
    tft.setCursor(x,y); tft.print("Battery: (stub)"); y += fontHeight() + 6;
    tft.setCursor(x,y); tft.print("Temp: (stub)");    y += fontHeight() + 6;

    float pct = min(1.0f, (millis()-start)/4000.0f);
    drawCenteredProgress(pct);

    updateRotary();
    if (isSelectPressed()) return;
    if (isBackPressed()) return;
    idleDimmerTick();
    delay(10);
  }
}

#endif
