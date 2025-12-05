#ifndef EEPROM_UTILS_H
#define EEPROM_UTILS_H

#include <Arduino.h>
#include <EEPROM.h>

// Pull project defaults. We still add safe fallbacks below so this header
// can compile even if config.h order changes.
#include "config.h"

// ---------- Fallback defaults (only used if not set in config.h) ----------
#ifndef EEPROM_SIZE
  #define EEPROM_SIZE       1024
#endif
#ifndef EEPROM_ADDR_BASE
  #define EEPROM_ADDR_BASE  0
#endif
#ifndef DEFAULT_MICROSTEPPING
  #define DEFAULT_MICROSTEPPING 16
#endif
#ifndef DEFAULT_CURRENT_MA
  #define DEFAULT_CURRENT_MA    600
#endif
#ifndef DEFAULT_STEPS_PER_REV
  #define DEFAULT_STEPS_PER_REV 200
#endif
#ifndef DEFAULT_PULLEY_TEETH
  #define DEFAULT_PULLEY_TEETH  20
#endif
#ifndef DEFAULT_BELT_PITCH_MM
  #define DEFAULT_BELT_PITCH_MM 2.0f
#endif
#ifndef DEFAULT_MOTION_PROFILE
  #define DEFAULT_MOTION_PROFILE 0
#endif
#ifndef DEFAULT_STOPS
  #define DEFAULT_STOPS 3
#endif
#ifndef DEFAULT_PAUSE_MS
  #define DEFAULT_PAUSE_MS 500
#endif

// ---------- Persistent types ----------
struct RuntimeState {
  uint16_t microstep       = DEFAULT_MICROSTEPPING;
  uint16_t current_mA      = DEFAULT_CURRENT_MA;
  uint16_t steps_per_rev   = DEFAULT_STEPS_PER_REV;
  uint16_t pulley_teeth    = DEFAULT_PULLEY_TEETH;
  float    belt_pitch_mm   = DEFAULT_BELT_PITCH_MM;

  uint8_t  motion_profile  = DEFAULT_MOTION_PROFILE; // 0=trap,1=smooth…
  uint8_t  defaultStops    = DEFAULT_STOPS;
  uint32_t defaultPauseMs  = DEFAULT_PAUSE_MS;

  bool     endpointsSaved  = false;
  float    endpointA_mm    = 0.0f;
  float    endpointB_mm    = 100.0f;
};

enum JobType : uint8_t { JOB_NONE=0, JOB_SINGLE=1, JOB_BOUNCE=2, JOB_MULTI=3, JOB_TIMELAPSE=4 };

struct LastJob {
  JobType  type           = JOB_NONE;
  uint16_t a_raw          = 0;
  uint16_t b_raw          = 0;
  bool     useTime        = true;
  uint32_t totalMS        = 60000;
  int      speedPct       = 50;
};

// ---------- Single-definition globals ----------
RuntimeState runtimeState;
LastJob      lastJob;

// ---------- EEPROM I/O ----------
static const uint32_t EEPROM_MAGIC = 0x534C4950; // 'SLIP'

// Call once in setup()
inline void eepromInit(){
  EEPROM.begin(EEPROM_SIZE);
}

// Save both runtimeState and lastJob
inline void eepromSaveRuntime(){
  uint16_t addr = EEPROM_ADDR_BASE;
  EEPROM.put(addr, EEPROM_MAGIC);   addr += sizeof(uint32_t);
  EEPROM.put(addr, runtimeState);   addr += sizeof(RuntimeState);
  EEPROM.put(addr, lastJob);        addr += sizeof(LastJob);
  EEPROM.commit();
}

// Load runtimeState + lastJob (with defaults if empty/corrupt)
inline void eepromLoadAllIntoRuntime(){
  uint16_t addr = EEPROM_ADDR_BASE;
  uint32_t magic = 0;
  EEPROM.get(addr, magic); addr += sizeof(magic);

  if (EEPROM_SIZE >= (EEPROM_ADDR_BASE + sizeof(magic) + sizeof(RuntimeState) + sizeof(LastJob))
      && magic == EEPROM_MAGIC)
  {
    EEPROM.get(addr, runtimeState); addr += sizeof(RuntimeState);
    EEPROM.get(addr, lastJob);      addr += sizeof(LastJob);

    // sanity rails
    if (runtimeState.microstep == 0 || runtimeState.microstep > 256)
      runtimeState.microstep = DEFAULT_MICROSTEPPING;
    if (runtimeState.current_mA < 200 || runtimeState.current_mA > 2000)
      runtimeState.current_mA = DEFAULT_CURRENT_MA;
    if (runtimeState.steps_per_rev == 0 || runtimeState.steps_per_rev > 2000)
      runtimeState.steps_per_rev = DEFAULT_STEPS_PER_REV;
    if (runtimeState.pulley_teeth < 8 || runtimeState.pulley_teeth > 120)
      runtimeState.pulley_teeth = DEFAULT_PULLEY_TEETH;
    if (runtimeState.belt_pitch_mm < 1.0f || runtimeState.belt_pitch_mm > 10.0f)
      runtimeState.belt_pitch_mm = DEFAULT_BELT_PITCH_MM;
  }
  else
  {
    runtimeState = RuntimeState();
    lastJob      = LastJob();
    eepromSaveRuntime();
  }
}

// Convenience: save “Single Slide” snapshot for “Previously Set”
inline void eepromSaveSingle(uint16_t a_raw, uint16_t b_raw, bool useTime, uint32_t totalMS, int speedPct){
  lastJob.type     = JOB_SINGLE;
  lastJob.a_raw    = a_raw;
  lastJob.b_raw    = b_raw;
  lastJob.useTime  = useTime;
  lastJob.totalMS  = totalMS;
  lastJob.speedPct = speedPct;
  eepromSaveRuntime();
}

#endif // EEPROM_UTILS_H
