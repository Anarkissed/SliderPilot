#pragma once
#include "ui_helpers.h"
#include "rotary_input.h"
#include "input_compat.h"

// Forward declarations for the REAL screens (no stubs)
extern void runSingleSlideWizard();
extern void runBounceSlideWizard();
extern void runMultiPositionWizard();
extern void runTimelapseWizard();
extern void runPreviouslySavedScreen();
extern void openManualMode();
extern void openSettingsMenu();
extern void openStatusScreen();

// Menu items
static const char* mainMenu[] = {
  "Single Slide",
  "Bounce Slide",
  "Multi-Position",
  "Timelapse Mode",
  "Previously Set",
  "Manual Mode",
  "Settings",
  "Status"
};
static const int MAIN_MENU_COUNT = sizeof(mainMenu)/sizeof(mainMenu[0]);

// Selection state
static int currentMenuIndex = 0;
static int firstVisible      = 0;   // first row shown in the 3-slot window

// Where the right rail starts (for the scroll+tabs area)
static inline int rightGutter() { return tft.width() - UI::RIGHT_COL_W - UI::PAD; }

// Compute y-top for row i in 3-visible layout, centered vertically
static inline void computeRowTop(int i, int& outYTop) {
  const int innerLeft  = UI::PAD;
  const int innerRight = rightGutter();
  (void)innerLeft; (void)innerRight;

  const int totalVisH = 3*UI::ITEM_H + 2*UI::GAP;
  const int cy = tft.height()/2;
  outYTop = cy - totalVisH/2 + i*(UI::ITEM_H + UI::GAP);
}

// Draw the whole menu
static void drawMainMenu() {
  uiBegin();
  drawRightTabTop("Back");
  drawRightTabBottom("Select");

  for (int vis=0; vis<3; ++vis) {
    int idx = firstVisible + vis;
    if (idx < 0 || idx >= MAIN_MENU_COUNT) continue;
    int yTop; computeRowTop(vis, yTop);
    drawListItemRailAware(yTop, mainMenu[idx], (idx == currentMenuIndex));
  }
  drawSlimScroll(MAIN_MENU_COUNT, firstVisible, 3);
}

// Handle input and enter sub-screens
static void handleMainMenu() {
  // encoder movement → one move per detent (rotary_input already debounced)
  static int lastPos = getRotaryPosition();
  int p = getRotaryPosition();
  if (p != lastPos) {
    int delta = p - lastPos;
    lastPos = p;
    currentMenuIndex = constrain(currentMenuIndex + (delta>0 ? 1 : -1), 0, MAIN_MENU_COUNT-1);

    // maintain 3-row window so selected stays centered when possible
    if (currentMenuIndex < firstVisible+1) firstVisible = max(0, currentMenuIndex-1);
    if (currentMenuIndex > firstVisible+1) firstVisible = min(MAIN_MENU_COUNT-3, currentMenuIndex-1);

    drawMainMenu();
  }

  // short Back = do nothing (you asked Back-short=scroll in submenus only)
  // long Back = exit program (or ignore here — we’ll ignore to stay on menu)
  if (isBackPressedLong()) {
    // optional: soft reset or show a small toast; for now, just repaint
    drawMainMenu();
  }

  // OK = launch selected
  if (isSelectPressed()) {
    switch (currentMenuIndex) {
      case 0: runSingleSlideWizard();     break;
      case 1: runBounceSlideWizard();     break;
      case 2: runMultiPositionWizard();   break;
      case 3: runTimelapseWizard();       break;
      case 4: runPreviouslySavedScreen(); break;
      case 5: openManualMode();           break;
      case 6: openSettingsMenu();         break;
      case 7: openStatusScreen();         break;
      default: break;
    }
    // redraw menu when coming back
    drawMainMenu();
  }
}