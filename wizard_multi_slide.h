#pragma once
#include "ui_helpers.h"
#include "rotary_input.h"

inline void runMultiPositionWizard(){
  uiBegin();
  drawRightTabTop("Back");
  drawRightTabBottom("OK");
  tft.setTextDatum(MC_DATUM); fontTitle();
  tft.drawString("Multi-Position", tft.width()/2, tft.height()/2 - 14);
  fontBody(); tft.drawString("Coming next", tft.width()/2, tft.height()/2 + 12);
  tft.setTextDatum(TL_DATUM);
  while (!isSelectPressed() && !isBackPressedLong()){ pollInput(); delay(10); }
}