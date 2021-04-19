/**
 * SeeVersion.cpp
 */

#include "SeeVersion.h"

#include "../TankControllerLib.h"
#include "Devices/LiquidCrystal_TC.h"

void SeeVersion::start() {
  LiquidCrystal_TC::instance()->writeLine(prompt(), 0);
  const char* version = TankControllerLib::instance()->version();
  LiquidCrystal_TC::instance()->writeLine(version, 1);
}
