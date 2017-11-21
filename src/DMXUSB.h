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

class DMXUSB {
  public:
    DMXUSB(usb_serial_class serial, int baudrate, int mode, void (*dmxInCallback)(int universe, unsigned int index, char buffer[512]));
    void listen();
    void init();
  private:
    char _buffer[512];
    elapsedMillis _timeout;
    usb_serial_class _serial;
    int _baudrate;
    int _mode;
    void (*_dmxInCallback)(int universe, unsigned int index, char buffer[512]);
};

#endif

