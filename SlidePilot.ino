#include <Arduino.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();       // single definition

#include "config.h"
#include "ui_helpers.h"

// Your other modules
#include "rotary_input.h"
#include "menu.h"
// If you use these, keep them included as before:
// #include "wizard_single_slide.h"
// #include "wizard_bounce_slide.h"
// #include "wizard_multi_slide.h"
// #include "wizard_timelapse.h"
// #include "settings_menu.h"
// #include "status_screen.h"
// #include "previous_slide.h"
// #include "manual_mode.h"

void setup() {
  // Keep GPIO15 safely low at boot (still keep the physical 10kΩ to GND)
  pinMode(15, INPUT_PULLDOWN);
  delay(5);

  // Backlight first
  backlightInit();
  backlightSet(BRIGHT_LEVEL);

  // TFT init
  tft.begin();
  tft.setRotation(1);

  // UI base
  uiBegin();

  // Inputs
  inputInit();   // from rotary_input.h (sets up CLK/DT/OK/BACK)

  // Draw main menu on boot
  drawMainMenu();
}

void loop() {
  // Read encoder/buttons
  handleRotary();   // or updateRotary()/pollInput() if that’s what your project uses

  // Menu state machine
  handleMainMenu();

  // Optional UI idle/dimmer (currently no-op)
  idleDimmerTick();
}