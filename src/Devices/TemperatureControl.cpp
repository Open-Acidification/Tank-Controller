#include "TemperatureControl.h"

#include "Devices/DateTime_TC.h"
#include "Devices/EEPROM_TC.h"
#include "Serial_TC.h"
#include "TankControllerLib.h"

const float DEFAULT_TEMPERATURE = 20.0;

//  class instance variables
/**
 * static variable for singleton
 */
TemperatureControl *TemperatureControl::_instance = nullptr;

//  class methods
/**
 * static member function to return singleton
 */
TemperatureControl *TemperatureControl::instance() {
  if (!_instance) {
    if (EEPROM_TC::instance()->getHeat()) {
      _instance = new Heater();
    } else {
      _instance = new Chiller();
    }
  }
  return _instance;
}

void TemperatureControl::enableHeater(bool flag) {
  EEPROM_TC::instance()->setHeat(flag);
  if (_instance && (_instance->isHeater() != flag)) {
    delete _instance;
    _instance = nullptr;
    serial("TemperatureControl::enableHeater(%s)", flag ? "true" : "false");
    instance();
  }
}

/**
 * protected constructor
 */
TemperatureControl::TemperatureControl() {
  COUT("TemperatureControl()");
  targetTemperature = EEPROM_TC::instance()->getTemp();
  if (isnan(targetTemperature)) {
    targetTemperature = DEFAULT_TEMPERATURE;
    EEPROM_TC::instance()->setTemp(targetTemperature);
  }
  pinMode(TEMP_CONTROL_PIN, OUTPUT);
  digitalWrite(TEMP_CONTROL_PIN, TURN_SOLENOID_OFF);
  serial("%s starts with solenoid off with target temperature of %5.2f C", this->isHeater() ? "Heater" : "Chiller",
         targetTemperature);
}

/**
 * is heater
 */
bool TemperatureControl::isHeater() {
  return true;
}

bool TemperatureControl::isOn() {
  return digitalRead(TEMP_CONTROL_PIN) == TURN_SOLENOID_ON;
}

/**
 * set target temperature and save in EEPROM
 */
void TemperatureControl::setTargetTemperature(float newTemperature) {
  if (targetTemperature != newTemperature) {
    serial("Change target temperature from %5.2f to %5.2f", targetTemperature, newTemperature);
    EEPROM_TC::instance()->setTemp(newTemperature);
    targetTemperature = newTemperature;
  }
}

void Chiller::updateControl(float currentTemperature) {
  uint32_t currentMillis = millis();
  COUT("Chiller::updateControl(" << currentTemperature << ") at " << currentMillis);
  if (currentMillis < previousMillis) {
    COUT("Reset previousMillis from " << previousMillis << " to 0");
    previousMillis = 0;  // reset if clock went backwards (typical during tests)
  }
  // pause 30 seconds between switching chiller on and off to prevent damage to chiller
  if (currentMillis - previousMillis < TIME_INTERVAL) {
    COUT("Chiller update at " << currentMillis << " ignored due to update at " << previousMillis);
  } else {
    bool oldValue = digitalRead(TEMP_CONTROL_PIN);
    bool newValue;
    previousMillis = currentMillis;
    // if in calibration, turn unit off
    if (TankControllerLib::instance()->isInCalibration()) {
      newValue = TURN_SOLENOID_OFF;
      COUT("Chiller should be off");
    }
    // if the observed temperature is above the set-point range turn on the chiller
    else if (currentTemperature >= targetTemperature + DELTA) {
      newValue = TURN_SOLENOID_ON;
      COUT("Chiller should be on");
    }
    // if the observed temperature is below the set-point range turn off the chiller
    else if (currentTemperature <= targetTemperature - DELTA) {
      newValue = TURN_SOLENOID_OFF;
      COUT("Chiller should be off");
    } else {
      newValue = oldValue;
    }
    if (newValue != oldValue) {
      uint32_t currentMS = millis();
      serial("chiller turned %s at %lu after %lu ms", newValue ? "off" : "on", currentMS, currentMS - lastSwitchMS);
      lastSwitchMS = currentMS;
      digitalWrite(TEMP_CONTROL_PIN, newValue);
    }
  }
}

void Heater::updateControl(float currentTemperature) {
  COUT("Heater::updateControl(" << currentTemperature);
  bool oldValue = digitalRead(TEMP_CONTROL_PIN);
  bool newValue;
  // if in calibration, turn unit off
  if (TankControllerLib::instance()->isInCalibration()) {
    newValue = TURN_SOLENOID_OFF;
  }
  // if the observed temperature is below the temperature set-point range turn on the heater
  else if (currentTemperature <= targetTemperature - DELTA) {
    newValue = TURN_SOLENOID_ON;
  }
  // if the observed temperature is above the temperature set-point range turn off the heater
  else if (currentTemperature >= targetTemperature + DELTA) {
    newValue = TURN_SOLENOID_OFF;
  } else {
    newValue = oldValue;
    COUT("Heater update at " << millis() << " ignored due to recent change in state");
  }
  if (newValue != oldValue) {
    uint32_t currentMS = millis();
    serial("heater turned %s at %lu after %lu ms", newValue ? "off" : "on", currentMS, currentMS - lastSwitchMS);
    lastSwitchMS = currentMS;
    digitalWrite(TEMP_CONTROL_PIN, newValue);
  }
}
