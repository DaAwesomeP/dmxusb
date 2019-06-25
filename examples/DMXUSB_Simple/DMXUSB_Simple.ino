/* DMXUSB_Simple.ino
 * Originally created 11/21/2017 by Stefan Krüger (s-light)
 * This is a simple example sketch for the DMXUSB Arduino/Teensy library.
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

const byte LED_PIN = 13;

// DMXUSB should receive and transmit data at the highest, most reliable speed possible
// Recommended Arduino baud rate: 115200 (limits maximum framerate and/or number of channels
// Recommended Teensy USB baud rate: 12000000 (12 Mb/s)
// DMX baud rate: 250000
// MIDI baud rate: 31250
#define DMXUSB_BAUDRATE 115200

// receive a DMX transmission
void myDMXCallback(int universe, char buffer[512]) {
  for (int index=0; index < 512; index++) { // for each channel, universe starts at 0
    int channel = index + 1; // channel starts at 0, so index 0 is DMX channel 1 and index 511 is DMX channel 512
    int value = buffer[index]; // DMX value 0 to 255
    if (universe == 0 && channel == 1) analogWrite(LED_PIN, value); // LED on channel 1 on universe 0
  }
}

DMXUSB myDMXUsb(
  // Stream serial,
  Serial,
  // int baudrate,
  DMXUSB_BAUDRATE,
  // int mode,
  0,
  // void (*dmxInCallback)(int universe, unsigned int index, char buffer[512])
  myDMXCallback
);

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(DMXUSB_BAUDRATE);
}

void loop() {
  myDMXUsb.listen();
}
