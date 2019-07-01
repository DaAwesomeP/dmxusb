/* DMXUSB.cpp
 * Originally created 11/20/2017 by the Perry Naseck (DaAwesomeP)
 * This is the source file to the DMXUSB Arduino/Teensy library.
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

#include "Arduino.h"
#include "DMXUSB.h"

// Byte order for Enttec/DMXKing protocol
#define STATE_START    0
#define STATE_LABEL    1
#define STATE_LEN_LSB  2
#define STATE_LEN_MSB  3
#define STATE_DATA     4
#define STATE_END      5

// Initialize DMXUSB serial port
DMXUSB::DMXUSB(Stream &serial, int baudrate, int mode, void (*dmxInCallback)(int universe, char buffer[512]), int outUniverses, uint8_t serialNum[]) {
  _serial = &serial;
  _baudrate = baudrate;
  _mode = mode;
  _dmxInCallback = dmxInCallback;
  if (_mode == 0) _outUniverses = 1;
  else if (_mode == 1) _outUniverses = 2;
  else if (_mode == 2) _outUniverses = outUniverses;
  #if defined(AUTO_SERIAL_AVAILABLE)
    if (serialNum[0] == 0xff && serialNum[0] == 0xff && serialNum[0] == 0xff && serialNum[0] == 0xff) {
      DMXUSB::teensySerial();
    } else _serialNum = serialNum;
  #else
    _serialNum = serialNum;
  #endif
}

// Get Teensy Serial Number for LC, 3.0, 3.1/3.2, 3.5, and 3.6
// Taken from TeensyMAC, modified to modify a uint8_t instead of return a uint32_t:
//   https://github.com/FrankBoesing/TeensyMAC/blob/a5b394bd91a0740bc4d974f7174eb426853a9ddd/TeensyMAC.cpp
#define MY_SYSREGISTERFILE	((uint8_t *)0x40041000) // System Register File
uint32_t DMXUSB::_getserialhw(void) {
	uint32_t num;
	__disable_irq();
  #if defined(HAS_KINETIS_FLASH_FTFA) || defined(HAS_KINETIS_FLASH_FTFL)
    FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
    FTFL_FCCOB0 = 0x41;
    FTFL_FCCOB1 = 15;
    FTFL_FSTAT = FTFL_FSTAT_CCIF;
    while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) ; // wait
    num = *(uint32_t *)&FTFL_FCCOB7;
  #elif defined(HAS_KINETIS_FLASH_FTFE)
    // Does not work in HSRUN mode :
    FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
    *(uint32_t *)&FTFL_FCCOB3 = 0x41070000;
    FTFL_FSTAT = FTFL_FSTAT_CCIF;
    while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) ; // wait
    num = *(uint32_t *)&FTFL_FCCOBB;
  #endif
	__enable_irq();
	return num;
}
#if defined(HAS_KINETIS_FLASH_FTFE) && (F_CPU > 120000000)
  extern "C" void startup_early_hook(void) {
    #if defined(KINETISK)
      WDOG_STCTRLH = WDOG_STCTRLH_ALLOWUPDATE;
    #elif defined(KINETISL)
      SIM_COPC = 0;  // disable the watchdog
    #endif
     *(uint32_t*)(MY_SYSREGISTERFILE) = DMXUSB::_getserialhw();
  }

  void DMXUSB::teensySerial(void) {
    uint32_t num;
    num = *(uint32_t*)(MY_SYSREGISTERFILE);
    // add extra zero to work around OS-X CDC-ACM driver bug
    // http://forum.pjrc.com/threads/25482-Duplicate-usb-modem-number-HELP
    if (num < 10000000) num = num * 10;
    *(uint32_t*)_serialNum = num;
  }
#else
	void DMXUSB::teensySerial(void) {
    uint32_t num;
    num = DMXUSB::_getserialhw();
    // add extra zero to work around OS-X CDC-ACM driver bug
    // http://forum.pjrc.com/threads/25482-Duplicate-usb-modem-number-HELP
    if (num < 10000000) num = num * 10;
    *(uint32_t*)_serialNum = num;
  }
#endif

// Poll for incoming DMX messages
// Modified basic Enttec emulation code from Paul Stoffregen's code to include labels 77, 78, 100, and 101
// https://github.com/PaulStoffregen/Lighting_Controller/blob/master/electronics/CorePlay/CorePlay.ino
void DMXUSB::listen() {
  if (_serial->available()) {
    byte state = STATE_START;
    byte label = 0x00;
    unsigned int index = 0;
    unsigned int count = 0;
    byte b;
    _timeout = 0;

    while (1) {
      while (!_serial->available()) {
        // 0.5 seconds without data, reset state
        if (_timeout > 500) state = STATE_START;
      }
      _timeout = 0;
      b = _serial->read();

      switch (state) {
        // first bit: start of message
        case STATE_START:
          if (b == 0x7E) state = STATE_LABEL; // if start bit, move to second bit
          break;

        // second bit: message label
        case STATE_LABEL:
          label = b; // record the message label
          if (label == 6 || (label >= 100 && label < 100 + _outUniverses)) for (int i = 0; i < 512; i++) _buffer[i] = (byte)0x00; // set buffer to all zero values if receiving DMX data
          state = STATE_LEN_LSB; // move to next bit
          break;

        // third bit: data length LSB (0 to 600)
        case STATE_LEN_LSB:
          count = b; // record the message length
          state = STATE_LEN_MSB; // move to next bit
          break;

        // fourth bit: data length MSB
        case STATE_LEN_MSB:
          count |= (b << 8);
          index = 0;
          if (count > 0) {
            state = STATE_DATA;
          } else {
            state = STATE_END;
          }
          break;

        // starting at fifth bit: data
        case STATE_DATA:
          if (index <= sizeof(_buffer)) { // include 512
            if (index > 0) {
              _buffer[index - 1] = b; // record the data, DMX channels start at 1 but array of channels starts at 0
            }
            index++;
          }
          count = count - 1; // decrease the data count
          if (count == 0) {
            state = STATE_END; // if no more data, move on to last bit
          }
          break;

        // final bit
        case STATE_END:
          if (b == 0xE7) { // if final bit
            state = STATE_START;
            if (label == 77) { // if message is of label 77 (ETSA ID request), then send ETSA ID
              int len = 8;
              _serial->write(0x7E); // message header
              _serial->write(0x4D); // label 77
              _serial->write(len & 0xff); // data length LSB: 2 + MSB
              _serial->write((len + 1) >> 8); // data length MSB
              if (_mode == 0 || _mode == 1) { // DMXKing device
                _serial->write(0x6B);
                _serial->write(0x6A);
              } else if (_mode == 2) { // DMXUSB device
                _serial->write(0xF7); // Uses ETSA prototype ID
                _serial->write(0x7F); // Uses ETSA prototype ID
              }
              _serial->write("DMXUSB");
              _serial->write(0xE7); // message footer
            }

            else if (label == 78) { // if message is of label 78 (device ID request), then send device ID
              int len = 2;
              if (_mode == 0) len += 23;
              if (_mode == 1) len += 21;
              if (_mode == 2) len += 28;
              _serial->write(0x7E); // message header
              _serial->write(0x4E); // label 78
              _serial->write(len & 0xff); // data length LSB: 2
              _serial->write((len + 1) >> 8); // data length MSB: 0
              if (_mode == 0) { // DMXKing ultraDMX Micro (one universe, Enttec compatible)
                _serial->write(0x03); // id 3
                _serial->write((byte)0x00);
                _serial->write("Emulated ultraDMX Micro");
              } else if (_mode == 1) { // DMXKing ultraDMX Pro (two universes, Enttec compatible with label 6)
                _serial->write(0x02); // id 2
                _serial->write((byte)0x00);
                _serial->write("Emulated UltraDMX Pro");
              } else if (_mode == 2) { // DMXUSB device (many universes, Enttec compatible with label 6)
                _serial->write(0x32); // id 50 (high to avoid possible collisions since ETSA ID isn't reserved)
                _serial->write((byte)0x00);
                _serial->write("DMXUSB Multi-universe Device");
              }
              _serial->write(0xE7); // message footer
            }

            else if (label == 10) { // if message is of label 10 (serial number request), then send device serial number
              int len = 4;
              _serial->write(0x7E); // message header
              _serial->write(0x0A); // label 10
              _serial->write(len & 0xff); // data length LSB: 4
              _serial->write((len + 1) >> 8); // data length MSB: 0
              _serial->write(_serialNum[3]);
              _serial->write(_serialNum[2]);
              _serial->write(_serialNum[1]);
              _serial->write(_serialNum[0]);
              _serial->write(0xE7); // message footer
            }

            else if (label == 3) { // if message is of label 3 (widget parameters request), then send widget parameters
              int len = 5;
              _serial->write(0x7E); // message header
              _serial->write(0x03); // label 3
              _serial->write(len & 0xff); // data length LSB: 4
              _serial->write((len + 1) >> 8); // data length MSB: 0
              _serial->write((byte)0x03); // firmware version LSB: 3 (v0.0.4)
              _serial->write((byte)0x00); // firmware version MSB: 0
              _serial->write(0x09); // DMX output break time in 10.67 microsecond units: 9 (TODO: CALCUALTE WITH BAUDRATE)
              _serial->write(0x01); // DMX output Mark After Break time in 10.67 microsecond units: 1 (TODO: CALCUALTE WITH BAUDRATE)
              _serial->write(0x28); // DMX output rate in packets per second: 40 (TODO: CALCUALTE WITH BAUDRATE)
              _serial->write(0xE7); // message footer
            }

            else if (label == 53 && _mode == 2) { // if message is of label 203 (extended widget parameters request), then send extended widget parameters
              int len = 2;
              _serial->write(0x7E); // message header
              _serial->write(0x35); // label 53
              _serial->write(len & 0xff); // data length LSB: 2
              _serial->write((len + 1) >> 8); // data length MSB: 0
              // _serial->write((byte)0x00); // out universes
              _serial->write((byte)_outUniverses); // out universes
              _serial->write((byte)0x00); // in universes (not implemented)
              _serial->write(0xE7); // message footer
            }

            else if (label == 6 || (label >= 100 && label < 100 + _outUniverses)) { // receive DMX message to all universes
              //if (index > 1) {
              if (label == 6 && _mode == 0) this->DMXUSB::_dmxInCallback(0, _buffer); // receive label==6 DMX message to first universe for Enttec-like ultraDMX Micro device
              else if (label == 6 && _mode == 1) {  // receive label==6 DMX message to both universes for ultraDMX Pro device
                this->DMXUSB::_dmxInCallback(0, _buffer);
                this->DMXUSB::_dmxInCallback(1, _buffer);
              } else if (label == 6 && _mode == 2) {  // receive label==6 DMX message to all universes for DMXUSB device
                for (int i = 0; i < _outUniverses; i++) this->DMXUSB::_dmxInCallback(i, _buffer);
              } else if (label == 100 && _mode == 1) this->DMXUSB::_dmxInCallback(0, _buffer); // receive label==100 DMX message to first universe for ultraDMX Pro device
              else if (label == 101 && _mode == 1) this->DMXUSB::_dmxInCallback(1, _buffer); // receive label==101 DMX message to second universe for ultraDMX Pro device
              else if (_mode == 2) this->DMXUSB::_dmxInCallback(label - 100, _buffer); // receive labels 100 through 107 DMX message to each universe for DMXUSB device
              //}
            }
          }
          break;

        default:
          state = STATE_START;
          break;
      }
    }
  }
}
