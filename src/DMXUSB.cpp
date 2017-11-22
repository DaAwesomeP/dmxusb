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
DMXUSB::DMXUSB(Stream &serial, int baudrate, int mode, void (*dmxInCallback)(int universe, unsigned int index, char buffer[512])) {
  _serial = &serial;
  _baudrate = baudrate;
  _mode = mode;
  _dmxInCallback = dmxInCallback;
}

// Poll for incoming DMX messages
// Modified basic Enttec emulation code from Paul Stoffregen's code to include labels 77, 78, 100, and 101
// https://github.com/PaulStoffregen/Lighting_Controller/blob/master/electronics/CorePlay/CorePlay.ino
void DMXUSB::listen() {
  if (_serial->available()) {
    byte state = STATE_START;
    byte label;
    unsigned int index, count;
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
            _buffer[index - 1] = b; // record the data, DMX channels start at 1 but array of channels starts at 0
            index++;
          }
          count = count - 1; // decrease the data count
          if (count == 0) {
            //for (int i = 512; i >= (index-1); i--) _buffer[i] = (byte)0x00; // set unsent values to 0 // for all unsent values (protocol says 0 values are omitted after last value)
            state = STATE_END; // if no more data, move on to last bit
          }
          break;

        // final bit
        case STATE_END:
          if (b == 0xE7) { // if final bit
            state = STATE_START;
            if (label == 77) { // if message is of label 77 (ETSA ID request), then send ETSA ID
              int len = 2;
              _serial->write(0x7E); // message header
              _serial->write(0x4D); // label 77
              _serial->write(len & 0xff); // data length LSB: 2 + MSB
              _serial->write((len + 1) >> 8); // data length MSB
              if (_mode == 0 || _mode == 1) { // DMXKing device
                _serial->write(0x6B);
                _serial->write(0x6A);
              }
              //_serial->write("Teensy DMX");
              _serial->write(0xE7); // message footer
            }

            else if (label == 78) { // if message is of label 78 (device ID request), then send device ID
              int len = 2;
              _serial->write(0x7E); // message header
              _serial->write(0x4E); // label 78
              _serial->write(len & 0xff); // data length LSB: 2
              _serial->write((len + 1) >> 8); // data length MSB: 0
              if (_mode == 0) { // DMXKing ultraDMX Micro (one universe, Enttec compatible)
                _serial->write(0x03); // id 3
                _serial->write((byte)0x00);
              } else if (_mode == 1) { // DMXKing ultraDMX Pro (two universes, Enttec compatible with label 6)
                _serial->write(0x02); // id 2
                _serial->write((byte)0x00);
              }
              //_serial->write("Fake UltraDMX Pro");
              _serial->write(0xE7); // message footer
            }

            else if (label == 10) { // if message is of label 10 (serial number request), then send device serial number
              int len = 4;
              _serial->write(0x7E); // message header
              _serial->write(0x0A); // label 10
              _serial->write(len & 0xff); // data length LSB: 4
              _serial->write((len + 1) >> 8); // data length MSB: 0
              _serial->write(0xFF); // for now just use serial number 0xFFFFFFFF
              _serial->write(0xFF);
              _serial->write(0xFF);
              _serial->write(0xFF);
              _serial->write(0xE7); // message footer
            }

            else if (label == 3) { // if message is of label 3 (widget parameters request), then send widget parameters
              int len = 5;
              _serial->write(0x7E); // message header
              _serial->write(0x03); // label 3
              _serial->write(len & 0xff); // data length LSB: 4
              _serial->write((len + 1) >> 8); // data length MSB: 0
              _serial->write((byte)0x00); // firmware version LSB: 0
              _serial->write((byte)0x00); // firmware version MSB: 0
              _serial->write(0x09); // DMX output break time in 10.67 microsecond units: 9 (TODO: CALCUALTE WITH BAUDRATE)
              _serial->write(0x01); // DMX output Mark After Break time in 10.67 microsecond units: 1 (TODO: CALCUALTE WITH BAUDRATE)
              _serial->write(0x28); // DMX output rate in packets per second: 40 (TODO: CALCUALTE WITH BAUDRATE)
              _serial->write(0xE7); // message footer
            }

            else if (label == 6 || label == 100 || label == 101) { // receive DMX message to both universes
              //if (index > 1) {
              if (label == 6 && _mode == 0) this->DMXUSB::_dmxInCallback(0, index, _buffer); // receive label==6 DMX message to first universe for Enttec-like ultraDMX Micro device
              if (label == 6 && _mode == 1) {  // receive label==6 DMX message to both universes for ultraDMX Pro device
                this->DMXUSB::_dmxInCallback(0, index, _buffer);
                this->DMXUSB::_dmxInCallback(1, index, _buffer);
              }
              if (label == 100 || _mode == 1) this->DMXUSB::_dmxInCallback(0, index, _buffer); // receive label==100 DMX message to first universe for ultraDMX Pro device
              if (label == 101 || _mode == 1) this->DMXUSB::_dmxInCallback(1, index, _buffer); // receive label==101 DMX message to second universe for ultraDMX Pro device
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

