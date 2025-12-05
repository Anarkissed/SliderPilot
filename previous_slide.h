#ifndef PREVIOUS_SLIDE_H
#define PREVIOUS_SLIDE_H

#include "wizard_ui.h"
#include "ui_helpers.h"
#include "rotary_input.h"

inline void runPreviouslySavedScreen() {
  while (true) {
    wizardFrameStart("Begin");
    wizardCenterTwo("Previously Set", "No saved data");

    updateRotary();
    if (isSelectPressed()) return; // Begin would start; here just return
    if (isBackPressed()) return;
    idleDimmerTick();
    delay(10);
  }
}

#endif
