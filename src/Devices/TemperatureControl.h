#pragma once
#include <Arduino.h>

#include "TankControllerLib.h"

/**
 * Temperature Control
 * Controlling the tank temperature is done by external devices that are controlled by setting
 * designated pins on the Arduino.
 */

class TemperatureControl {
private:
  static TemperatureControl* _instance;

protected:
  const uint16_t TEMP_CONTROL_PIN = 47;
  const float DELTA = 0.05;
  uint32_t lastSwitchMS = 0;
  float targetTemperature;
  TemperatureControl();

public:
  virtual ~TemperatureControl() {
  }
  static TemperatureControl* instance();
  static void enableHeater(bool flag);
  float getTargetTemperature() {
    return targetTemperature;
  }
  virtual bool isHeater();
  bool isOn();
  void setTargetTemperature(float newTemperature);
  virtual void updateControl(float currentTemperature) = 0;
};

class Heater : public TemperatureControl {
public:
  Heater() : TemperatureControl(){};
  bool isHeater() {
    return true;
  }
  void updateControl(float currentTemperature);
};

class Chiller : public TemperatureControl {
private:
  const uint32_t TIME_INTERVAL = 30 * 1000UL;  // interval at which to change chiller state
  uint32_t previousMillis = 0;                 // will store last time chiller state was checked

public:
  Chiller() : TemperatureControl(){};
  bool isHeater() {
    return false;
  }
  void updateControl(float currentTemperature);
};
