/* DMXUSB_DmxSimple.ino
 * Originally created 11/25/2017 by Perry Naseck (DaAwesomeP)
 * This is an example sketch for the DMXUSB Arduino/Teensy library to create a USB to DMX device.
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

// NOTE: This code has not been tested. I do not own the hardware required to test it.
// More information about DmxSimple here: https://github.com/PaulStoffregen/DmxSimple

#include "DMXUSB.h"
#include <DmxSimple.h>

// DMXUSB should receive and transmit data at the highest, most reliable speed possible
// Recommended Arduino baud rate: 115200
// Recommended Teensy 3 baud rate: 2000000 (2 Mb/s)
// DMX baud rate: 250000
// MIDI baud rate: 31250
#define DMXUSB_BAUDRATE 115200

// Pin that DMX port is on
// DmxSimple uses pin 3 by default.
#define DMX_PIN 3

// Max DMX channels to output
// From the DmxSimple README:
//   "Timer 2 is used. Due to the small amount of RAM on 168 and Mega8 Arduinos, only
//   the first 128 channels are supported. Arduinos with processor sockets can easily
//   be upgraded to a 328 to control all 512 channels."
// This code has not been tested, so the performance is unknown. Please report your
// findings to https://github.com/DaAwesomeP/dmxusb/issues
#define DMX_MAX_CHANNELS 128

// receive a DMX transmission
void sendDMX(int universe, char buffer[512]) {
  for (int index=0; index < 512; index++) { // for each channel, universe starts at 0
    int channel = index + 1; // channel starts at 0, so index 0 is DMX channel 1 and index 511 is DMX channel 512
    int value = buffer[index]; // DMX value 0 to 255
    if (universe == 0 && channel <= DMX_MAX_CHANNELS) DmxSimple.write(channel, value); // Send universe 0 to the DMX port if in specified channel range
  }
}

DMXUSB DMXPC(
  // Stream serial,
  Serial,
  // int baudrate,
  DMXUSB_BAUDRATE,
  // int mode,
  0,
  // void (*dmxInCallback)(int universe, unsigned int index, char buffer[512])
  sendDMX
);

void setup() {
  Serial.begin(DMXUSB_BAUDRATE);

  // DMX port
  DmxSimple.usePin(DMX_PIN);

  // Number of channels to output on
  DmxSimple.maxChannel(DMX_MAX_CHANNELS);
}

void loop() {
  DMXPC.listen();
}

