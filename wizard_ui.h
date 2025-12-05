#ifndef WIZARD_UI_H
#define WIZARD_UI_H

#include "ui_helpers.h"
#include "rotary_input.h"

// Draw “Back” (top tab) + “OK” (bottom tab), and clear background.
inline void wizardFrameStart(const char* okLabel = "OK") {
  uiBegin();
  drawRightTabTop("Back");
  drawRightTabBottom(okLabel ? okLabel : "OK");
}

// Center 1–2 lines in the content area (big and readable)
inline void wizardCenterTwo(const char* line1, const char* line2 = nullptr) {
  const int innerTop = 8;
  const int innerBot = tft.height() - 8;
  const int innerH   = innerBot - innerTop;

  tft.setTextDatum(MC_DATUM);
  fontBody();
  tft.setTextColor(Theme::TEXT, Theme::BG);

  if (line2 && *line2) {
    int y1 = innerTop + innerH/2 - fontHeight();
    int y2 = innerTop + innerH/2 + fontHeight()/2 + 4;
    tft.drawString(line1, tft.width()/2 - UI::RIGHT_COL_W/2, y1);
    tft.drawString(line2, tft.width()/2 - UI::RIGHT_COL_W/2, y2);
  } else {
    tft.drawString(line1, tft.width()/2 - UI::RIGHT_COL_W/2, innerTop + innerH/2);
  }

  tft.setTextDatum(TL_DATUM);
}

#endif
