#ifndef LOGO_H
#define LOGO_H

#include "ui_helpers.h"

inline void playStartupAnimation() {
  uiBegin();
  drawHeader("SlidePilot");
  int y = tft.height()/2;
  uint32_t start = millis();
  while (millis() - start < 1200) {
    tft.fillRect(0, 26, tft.width(), tft.height()-46, Theme::BG);
    // Title
    tft.setTextColor(Theme::TEXT, Theme::BG);
    tft.setTextFont(4);
    int16_t cx = 10; int16_t cy = y-12;
    tft.setCursor(cx, cy); tft.print("SlidePilot");
    tft.setTextFont(2);
    tft.setCursor(cx, cy+24); tft.setTextColor(Theme::TEXT_DIM, Theme::BG); tft.print("Camera Slider");

    // Moving slider icon under text
    drawMovingSliderIcon(cy+42 + 10, Theme::SEP, Theme::PRIMARY, 40, (millis()/30));

    drawFooter("Back", ""); // hint area
    delay(30);
  }
  tft.setTextFont(2);
}

#endif
