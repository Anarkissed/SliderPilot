#ifndef MANUAL_MODE_H
#define MANUAL_MODE_H

#include "wizard_ui.h"
#include "ui_helpers.h"
#include "rotary_input.h"

inline void openManualMode() {
  while (true) {
    wizardFrameStart("OK");
    wizardCenterTwo("Manual Mode", "Use knobs/buttons (stub)");

    updateRotary();
    if (isSelectPressed()) return;
    if (isBackPressed()) return;
    idleDimmerTick();
    delay(10);
  }
}

#endif
