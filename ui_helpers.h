#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"

// fonts (built-in, simple & consistent)
inline void fontBody()  { tft.setTextFont(2); }
inline void fontLabel() { tft.setTextFont(2); }

// cheap text metrics (avoid getTextBounds due to builds)
inline int textWidth(const char* s){ return (int)strlen(s) * 12; }
inline int fontHeight(){ return 16; }

// main UI clear
inline void uiBegin(){
  tft.fillScreen(Theme::BG);
  tft.setTextColor(Theme::TEXT, Theme::BG);
  fontBody();
}

// optional no-op dimmer
inline void idleDimmerTick(){}

// Slim right scrollbar, sized for a 3-row list area
inline void drawSlimScroll(int totalItems, int firstVisible, int visibleRows){
  int railX = tft.width() - UI::RIGHT_COL_W/2 - 4; // centered in right column
  int railH = tft.height() - 2*UI::PAD;
  int railY = UI::PAD;

  // rail
  tft.fillRoundRect(railX-4, railY, 8, railH, 4, Theme::ELEV_2);

  // compute thumb
  int thumbH = max(22, (railH * visibleRows) / max(visibleRows, totalItems));
  int maxFirst = max(0, totalItems - visibleRows);
  int thumbY = railY + ((maxFirst==0)?0 : (railH - thumbH) * clampT(firstVisible,0,maxFirst) / max(1,maxFirst));

  tft.fillRoundRect(railX-4, thumbY, 8, thumbH, 4, Theme::ACCENT);
}

// pill list item respecting right rail
inline void drawPillTextCentered(int yTop, const char* label, bool selected) {
  int left = UI::PAD;
  int right = tft.width() - UI::RIGHT_COL_W - UI::PAD;
  int w = right - left;
  int h = UI::ITEM_H;

  uint16_t bg = selected ? Theme::ACCENT : Theme::ELEV_1;
  uint16_t fg = selected ? Theme::BG : Theme::TEXT;

  tft.fillRoundRect(left, yTop, w, h, h/2, bg);

  fontBody();
  tft.setTextColor(fg, bg);
  int tw = textWidth(label);
  int cx = left + (w - tw)/2;
  int cy = yTop + (h - fontHeight())/2 + 2;
  tft.setCursor(cx, cy);
  tft.print(label);
}

// public wrapper used by menus/wizards
inline void drawListItemRailAware(int yTop, const char* label, bool selected){
  drawPillTextCentered(yTop, label, selected);
}

// centered progress (nice bar)
inline void drawCenteredProgress(int pct){
  int left = UI::PAD;
  int right = tft.width() - UI::RIGHT_COL_W - UI::PAD;
  int w = right - left;
  int h = 18;
  int y = (tft.height() - h)/2;

  tft.fillRoundRect(left-1, y-1, w+2, h+2, h/2, Theme::ELEV_2);
  int fillW = (w * clampT(pct,0,100))/100;
  tft.fillRoundRect(left, y, fillW, h, h/2, Theme::ACCENT);
}

// Right-side “tab” hints some screens still call.
// Top tab (Back)
inline void drawRightTabTop(const char* txt){
  int tabW = UI::RIGHT_COL_W - 6;
  int x = tft.width() - UI::RIGHT_COL_W + 3;
  int y = UI::PAD;
  int h = 20;
  // simple rounded pill
  tft.fillRoundRect(x, y, tabW, h, 6, Theme::ELEV_2);
  fontLabel();
  tft.setTextColor(Theme::TEXT, Theme::ELEV_2);
  int tw = textWidth(txt);
  int cx = x + (tabW - tw)/2;
  int cy = y + (h - fontHeight())/2 + 2;
  tft.setCursor(cx, cy);
  tft.print(txt);
}

// Bottom tab (Select)
inline void drawRightTabBottom(const char* txt){
  int tabW = UI::RIGHT_COL_W - 6;
  int x = tft.width() - UI::RIGHT_COL_W + 3;
  int h = 20;
  int y = tft.height() - UI::PAD - h;
  tft.fillRoundRect(x, y, tabW, h, 6, Theme::ELEV_2);
  fontLabel();
  tft.setTextColor(Theme::TEXT, Theme::ELEV_2);
  int tw = textWidth(txt);
  int cx = x + (tabW - tw)/2;
  int cy = y + (h - fontHeight())/2 + 2;
  tft.setCursor(cx, cy);
  tft.print(txt);
}