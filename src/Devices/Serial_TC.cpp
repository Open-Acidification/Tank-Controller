#include "Devices/Serial_TC.h"

#include "DateTime_TC.h"
#include "SD_TC.h"
#include "TC_util.h"

/**
 * global serial() functions
 */
void serial(const char *format...) {
  va_list args;
  va_start(args, format);
  Serial_TC::instance()->vprintf(format, args);
  va_end(args);
}

/**
 * static variable for singleton
 */
Serial_TC *Serial_TC::_instance = nullptr;

/**
 * static member function to return singleton
 */
Serial_TC *Serial_TC::instance() {
  if (!_instance) {
    _instance = new Serial_TC();
  }
  return _instance;
}

/**
 * constructor (private so clients use the singleton)
 */
Serial_TC::Serial_TC() {
  Serial.begin(9600);
  // wait for Serial Monitor to connect. Needed for native USB port boards only:
  while (!Serial)
    ;
}

/**
 * printf() uses a variant of snprintf() so supports the expected formats
 */
void Serial_TC::vprintf(const char *format, va_list args) {
  char buffer[256];
  vsnprintf(buffer, sizeof(buffer), format, args);
  Serial.println(buffer);
  Serial.flush();
  // need to avoid recursion since SD_TC could call serial()
  if (!printIsActive) {
    printIsActive = true;
    // this seems to cause problems on the actual hardware
    // SD_TC::instance()->appendToLog(buffer);
    printIsActive = false;
  }
#ifdef MOCK_PINS_COUNT
#if DEBUG
  std::cout << "\tSERIAL: " << buffer << std::endl;
#endif
#endif
}
