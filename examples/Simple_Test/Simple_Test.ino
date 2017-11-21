/* Simple_Test.ino
 * Originally created 11/21/2017 by the Stefan Krüger (s-light)
 * This is on example usage for the  DMXUSB Arduino/Teensy library.
 * https://github.com/DaAwesomeP/dmxusb/
 *
 * Copyright 2017-present Stefan Krüger (s-light)
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

#include "DMXUSB.h"

DMXUSB DMXUSB(
  // usb_serial_class serial,
  Serial,
  // int baudrate,
  // Recommended Teensy 3 baud rate: 2000000 (2 Mb/s)
  // DMX baud rate: 250000
  // MIDI baud rate: 31250
  // Recommended Arduino baud rate: 115200
  115200,
  // int mode,
  0,
  // void (*dmxInCallback)(int universe, unsigned int index, char buffer[512])
  myDMXCallback
);


const byte LED_PIN = 13;

void myDMXCallback(int universe, unsigned int index, char buffer[512]) {
    // universe starts at 0
    unsigned int count;
    count = index;
    for (index=1; index <= count; index++) { // for each channel
      int channel = index;
      int value = buffer[index];
      analogWrite(LED_PIN, value); // not using channel for this example
    }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  myDMXUsb.init();
}

void loop() {
  myDMXUsb.listen();
}
