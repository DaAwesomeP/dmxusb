/* DMXUSB.h
 * Originally created 11/20/2017 by the Perry Naseck (DaAwesomeP)
 * This is the header file to the DMXUSB Arduino/Teensy library.
 * https://github.com/DaAwesomeP/dmxusb/
 *
 * Copyright 2017-present Perry Naseck (DaAwesomeP)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#ifndef DMXUSB_h
#define DMXUSB_h

#include "Arduino.h"
#if !defined(CORE_TEENSY)
  #include <elapsedMillis.h>
#endif

#if defined(__MK20DX128__)     // Teensy 3.0
    #define AUTO_SERIAL_AVAILABLE "3.0"
#elif defined(__MK20DX256__)   // Teensy 3.1 or 3.2
    #define AUTO_SERIAL_AVAILABLE "3.2"
#elif defined(__MKL26Z64__)    // Teensy LC
    #define AUTO_SERIAL_AVAILABLE "LC"
#elif defined(__MK64FX512__)   // Teensy 3.5
    #define AUTO_SERIAL_AVAILABLE "3.5"
#elif defined(__MK66FX1M0__)   // Teensy 3.6
    #define AUTO_SERIAL_AVAILABLE "3.6"
#endif

class DMXUSB {
  public:
    DMXUSB(Stream &serial, int baudrate, int mode, void (*dmxInCallback)(int universe, char buffer[512]), int outUniverses = 0, uint32_t serialNum = 0xffffffff);
    void listen();
    #if defined(AUTO_SERIAL_AVAILABLE)
      // Must be public to always run
      static uint32_t _getserialhw(void);
    #endif
  private:
    char _buffer[512];
    elapsedMillis _timeout;
    Stream *_serial;
    int _baudrate;
    int _mode;
    void (*_dmxInCallback)(int universe, char buffer[512]);
    int _outUniverses;
    #if defined(AUTO_SERIAL_AVAILABLE)
      void teensySerial(void);
    #endif
    uint32_t _serialNum;
};

#endif

