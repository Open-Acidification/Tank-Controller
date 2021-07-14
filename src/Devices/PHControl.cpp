#include "PHControl.h"

#include "Devices/DateTime_TC.h"
#include "Devices/EEPROM_TC.h"
#include "Devices/Serial_TC.h"
#include "PID_TC.h"
#include "TC_util.h"
#include "TankControllerLib.h"

const float DEFAULT_PH = 8.1;

//  class instance variables
/**
 * static variable for singleton
 */
PHControl *PHControl::_instance = nullptr;

//  class methods
/**
 * static member function to return singleton
 */
PHControl *PHControl::instance() {
  if (!_instance) {
    _instance = new PHControl();
  }
  return _instance;
}

PHControl::PHControl() {
  window_start_time = millis();
  pinMode(PH_CONTROL_PIN, OUTPUT);
  pinValue = TURN_SOLENOID_OFF;
  digitalWrite(PH_CONTROL_PIN, pinValue);
  targetPh = EEPROM_TC::instance()->getPH();
  if (isnan(targetPh)) {
    targetPh = DEFAULT_PH;
    EEPROM_TC::instance()->setPH(targetPh);
  }
  serial("PHControl with target pH = %5.3f", targetPh);
}

void PHControl::setTargetPh(float newPh) {
  if (targetPh != newPh) {
    DateTime_TC::now().printToSerial();
    serial("change target pH from %6.4f to %6.4f", targetPh, newPh);
    targetPh = newPh;
    EEPROM_TC::instance()->setPH(newPh);
  }
}

void PHControl::enablePID(bool flag) {
  usePID = flag;
  // save to EEPROM?
  DateTime_TC::now().printToSerial();
  serial((flag ? "enable PID" : "disable PID"));
}

bool PHControl::isOn() {
  return digitalRead(PH_CONTROL_PIN) == TURN_SOLENOID_ON;
}

void PHControl::updateControl(float pH) {
  if (usePID) {
    onTime = PID_TC::instance()->computeOutput(targetPh, pH);
  } else {
    onTime = pH > targetPh ? 10000 : 0;
  }
  long now = millis();
  while (window_start_time + 2 * WINDOW_SIZE < now) {
    window_start_time += WINDOW_SIZE;
  }
  if (now - window_start_time > WINDOW_SIZE) {  // time to shift the Relay Window
    window_start_time += WINDOW_SIZE;
    COUT("now: " << now << "; window_start_time: " << window_start_time);
  }
  COUT("target: " << targetPh << "; current: " << pH << "; onTime = " << onTime
                  << "; left = " << now - window_start_time);
  bool newValue = pinValue;
  if (TankControllerLib::instance()->isInCalibration()) {
    newValue = TURN_SOLENOID_OFF;  // turn off CO2 while in calibration
  } else if ((onTime > SOLENOID_OPENING_TIME) && (onTime >= (now - window_start_time))) {
    newValue = TURN_SOLENOID_ON;  // open CO2 solenoid
  } else {
    newValue = TURN_SOLENOID_OFF;  // close CO2 solenoid
  }
  if (newValue != pinValue) {
    pinValue = newValue;
    DateTime_TC::now().printToSerial();
    uint32_t currentMS = millis();
    serial("CO2 bubbler turned %s after %lu ms", pinValue ? "off" : "on", currentMS - lastSwitchMS);
    lastSwitchMS = currentMS;
    digitalWrite(PH_CONTROL_PIN, pinValue);
  }
}
