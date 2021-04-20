/**
 * SeePIDConstants.cpp
 */

#include "SeePIDConstants.h"

#include "Devices/LiquidCrystal_TC.h"
#include "Devices/PHProbe.h"
#include "Devices/PID_TC.h"
#include "TC_util.h"

void SeePIDConstants::loop() {
  int elapsedSeconds = ((millis() - startTime) / 3000) % 2;
  switch (elapsedSeconds) {
    case 0:
      loadKp(0);
      loadKi(1);
      break;
    case 1:
      loadKd(0);
      LiquidCrystal_TC::instance()->writeLine("               ", 1);
      break;
  }
}

void SeePIDConstants::loadKp(int line) {
  PID *pPID = PID_TC::instance()->getPID();
  char buffer[17];
  sprintf(buffer, "Kp: %.1f", pPID->GetKp());
  LiquidCrystal_TC::instance()->writeLine(buffer, line);
}

void SeePIDConstants::loadKi(int line) {
  PID *pPID = PID_TC::instance()->getPID();
  char buffer[17];
  sprintf(buffer, "Ki: %.1f", pPID->GetKi());
  LiquidCrystal_TC::instance()->writeLine(buffer, line);
}

void SeePIDConstants::loadKd(int line) {
  PID *pPID = PID_TC::instance()->getPID();
  char buffer[17];
  sprintf(buffer, "Kd: %.1f", pPID->GetKd());
  LiquidCrystal_TC::instance()->writeLine(buffer, line);
}
