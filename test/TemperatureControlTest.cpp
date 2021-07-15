#include <Arduino.h>
#include <ArduinoUnitTests.h>
#include <ci/ObservableDataStream.h>

#include "DateTime_TC.h"
#include "LiquidCrystal_TC.h"
#include "MainMenu.h"
#include "PHCalibrationMid.h"
#include "Serial_TC.h"
#include "TankControllerLib.h"
#include "TempProbe_TC.h"
#include "TemperatureControl.h"
/**
 * These tests test the UpdateControl virtual function for the heater and chiller subclass and
 * checks to see whether the heater or chiller should be in an on state or an off state.
 * The following command lets you skip all the other tests and only run these tests
 * bundle exec arduino_ci.rb --skip-examples-compilation --testfile-select=TemperatureControlTest.cpp
 */

const uint16_t TEMP_CONTROL_PIN = 47;
GodmodeState* state = GODMODE();
TankControllerLib* tc = TankControllerLib::instance();
LiquidCrystal_TC* lc = LiquidCrystal_TC::instance();
TempProbe_TC* tempProbe = TempProbe_TC::instance();
TemperatureControl* control;

unittest_setup() {
  state->resetClock();
  tempProbe->setTemperature(20);
  tc->setNextState(new MainMenu(tc), true);
  state->serialPort[0].dataOut = "";  // the history of data written
}

unittest_teardown() {
  tc->setNextState(new MainMenu(tc), true);
}

// Chiller
unittest(BeforeIntervalAndWithinDelta) {
  TemperatureControl::enableHeater(false);
  control = TemperatureControl::instance();
  control->setTargetTemperature(20);
  control->updateControl(20);
  assertEqual(TURN_SOLENOID_OFF, state->digitalPin[TEMP_CONTROL_PIN]);
  control->updateControl(20.04);
  assertEqual(TURN_SOLENOID_OFF, state->digitalPin[TEMP_CONTROL_PIN]);
}

// Chiller
unittest(BeforeIntervalAndOutsideDelta) {
  TemperatureControl::enableHeater(false);
  control = TemperatureControl::instance();
  control->setTargetTemperature(20);
  control->updateControl(20);
  assertEqual(TURN_SOLENOID_OFF, state->digitalPin[TEMP_CONTROL_PIN]);
  control->updateControl(20.05);
  assertEqual(TURN_SOLENOID_OFF, state->digitalPin[TEMP_CONTROL_PIN]);
}

// Chiller
unittest(AfterIntervalAndWithinDelta) {
  TemperatureControl::enableHeater(false);
  control = TemperatureControl::instance();
  control->setTargetTemperature(20);
  control->updateControl(20);
  assertEqual(TURN_SOLENOID_OFF, state->digitalPin[TEMP_CONTROL_PIN]);
  delay(31000);
  control->updateControl(20.04);
  assertEqual(TURN_SOLENOID_OFF, state->digitalPin[TEMP_CONTROL_PIN]);
}

/**
 * Test that is turned on when needed
 * \see unittest(disableChillerDuringCalibration)
 */
unittest(AfterIntervalAndOutsideDelta) {
  TemperatureControl::enableHeater(false);
  control = TemperatureControl::instance();
  control->setTargetTemperature(20);
  control->updateControl(20);
  DateTime_TC january(2021, 1, 15, 1, 48, 24);
  january.setAsCurrent();
  state->serialPort[0].dataOut = "";  // the history of data written
  // chiller is initially off and goes on when needed
  assertEqual(TURN_SOLENOID_OFF, state->digitalPin[TEMP_CONTROL_PIN]);
  assertEqual(6, millis());
  delay(31006);
  assertEqual(31012, millis());
  control->updateControl(20.05);
  assertTrue(control->isOn());
  assertEqual(TURN_SOLENOID_ON, state->digitalPin[TEMP_CONTROL_PIN]);
  assertEqual("chiller turned on at 31012 after 31012 ms\r\n", state->serialPort[0].dataOut);
  tc->loop();
  assertEqual("T=20.02 C 20.00 ", lc->getLines().at(1));
  state->serialPort[0].dataOut = "";  // the history of data written
  delay(31012);
  control->updateControl(19.95);
  assertFalse(control->isOn());
  assertEqual(TURN_SOLENOID_OFF, state->digitalPin[TEMP_CONTROL_PIN]);
  assertEqual("chiller turned off at 62031 after 31019 ms\r\n", state->serialPort[0].dataOut);
  tc->loop();
  assertEqual("T 20.02 c 20.00 ", lc->getLines().at(1));
}

/**
 * Test that chiller is not turned on during calibration
 * \see unittest(AfterIntervalAndOutsideDelta)
 */
unittest(disableChillerDuringCalibration) {
  TemperatureControl::enableHeater(false);
  control = TemperatureControl::instance();
  control->setTargetTemperature(20);
  control->updateControl(20);
  assertFalse(tc->isInCalibration());
  PHCalibrationMid* test = new PHCalibrationMid(tc);
  tc->setNextState(test, true);
  assertTrue(tc->isInCalibration());
  // chiller is initially off and stays off during calibration
  // (test is same as above)
  assertEqual(TURN_SOLENOID_OFF, state->digitalPin[TEMP_CONTROL_PIN]);
  delay(31000);
  control->updateControl(20.05);
  assertEqual(TURN_SOLENOID_OFF, state->digitalPin[TEMP_CONTROL_PIN]);
}

// Heater
unittest(WithinDelta) {
  TemperatureControl::enableHeater(true);
  control = TemperatureControl::instance();
  control->setTargetTemperature(20);
  control->updateControl(20);
  assertEqual(TURN_SOLENOID_OFF, state->digitalPin[TEMP_CONTROL_PIN]);
  control->updateControl(19.96);
  assertEqual(TURN_SOLENOID_OFF, state->digitalPin[TEMP_CONTROL_PIN]);
}

/**
 * Test that heater is turned on when needed
 * \see unittest(disableHeaterDuringCalibration)
 */
unittest(OutsideDelta) {
  TemperatureControl::enableHeater(true);
  control = TemperatureControl::instance();
  control->setTargetTemperature(20);
  control->updateControl(20);
  // heater is initially off, then turns on
  assertEqual(TURN_SOLENOID_OFF, state->digitalPin[TEMP_CONTROL_PIN]);
  control->updateControl(19.95);
  assertTrue(control->isOn());
  assertEqual(TURN_SOLENOID_ON, state->digitalPin[TEMP_CONTROL_PIN]);
  assertEqual("heater turned on at 6 after 6 ms\r\n", state->serialPort[0].dataOut);
  tc->loop();
  assertEqual("T 20.02 H 20.00 ", lc->getLines().at(1));
  state->serialPort[0].dataOut = "";  // the history of data written
  delay(300);
  control->updateControl(20.05);
  assertFalse(control->isOn());
  assertEqual(TURN_SOLENOID_OFF, state->digitalPin[TEMP_CONTROL_PIN]);
  assertEqual("heater turned off at 313 after 307 ms\r\n", state->serialPort[0].dataOut);
  tc->loop();
  assertEqual("T 20.02 h 20.00 ", lc->getLines().at(1));
}

/**
 * Test that heater is not turned on during calibration
 * \see unittest(OutsideDelta)
 */
unittest(disableHeaterDuringCalibration) {
  TemperatureControl::enableHeater(true);
  control = TemperatureControl::instance();
  assertFalse(tc->isInCalibration());
  PHCalibrationMid* test = new PHCalibrationMid(tc);
  tc->setNextState(test, true);
  assertTrue(tc->isInCalibration());
  // heater is initially off, and stays off due to calibration
  // (test is same as above)
  assertEqual(TURN_SOLENOID_OFF, state->digitalPin[TEMP_CONTROL_PIN]);
  control->setTargetTemperature(20);
  control->updateControl(19.95);
  assertEqual(TURN_SOLENOID_OFF, state->digitalPin[TEMP_CONTROL_PIN]);
}

unittest_main()
