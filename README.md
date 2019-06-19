DMXUSB
========
[![License](https://img.shields.io/github/license/DaAwesomeP/dmxusb.svg?style=flat-square)](https://github.com/DaAwesomeP/dmxusb/blob/master/LICENSE) [![GitHub release](https://img.shields.io/github/release/DaAwesomeP/dmxusb.svg?style=flat-square)](https://github.com/DaAwesomeP/dmxusb/releases/latest) [![Github Releases Downloads](https://img.shields.io/github/downloads/DaAwesomeP/dmxusb/latest/total.svg?style=flat-square)](https://github.com/DaAwesomeP/dmxusb/releases/latest) [![build status](https://img.shields.io/travis/DaAwesomeP/dmxusb.svg?maxAge=2592000&style=flat-square)](https://travis-ci.org/DaAwesomeP/dmxusb)
---
DMXUSB implements the ENTTEC DMX USB Pro Widget API Specification 1.44 on any serial port. DMXUSB can emulate a single DMX port/universe device like the DMXKing USB ultraDMX Micro or a two port/universe device like the DMXKing ultraDMX Pro. Both devices are compatible with the ENTTEC standard. DMXUSB works with the Open Lighting Architecture (OLA) as a usbserial device.

## Installation
DMXUSB is regularly tested on the Arduino Mega 2560 and the PJRC Teensy 3.2. Compilation tests for many more boards are [completed via Continuous Integration](https://travis-ci.org/DaAwesomeP/dmxusb).

### Via Arduino IDE Library Manager
In the Arduino IDE, go to `Sketch > Tools > Include Library > Manage Libraries` and search and install the latest release version DMXUSB. If you are not using a PJRC Teensy board, then you will also need to install the elapsedMillis library.

### Via the ZIP archive
Download the latest release ZIP from [here](https://github.com/DaAwesomeP/dmxusb/releases/latest) or the latest testing release from [here](https://github.com/DaAwesomeP/dmxusb/archive/master.zip). Then go to `Sketch > Tools > Include Library > Add ZIP Library`. If you are not using a PJRC Teensy board, then you will also need to download and install the [elapsedMillis library](https://github.com/pfeerick/elapsedMillis/releases/latest).

## Usage
Currently, the library only receives DMX messages from a PC over USB. Please take a look at the [`Simple_Test` sketch](examples/Simple_Test/Simple_Test.ino) for a complete example.

### DMXUSB (serial, baudrate, mode, callback)
The DMXUSB class initializes a new instance of the library. Example:
```cpp
DMXUSB myDMXUsb(Serial, 115200, 0, myDMXCallback);
```

#### serial (Stream)
Any Stream-based serial port. On my Arduino-like boards, `Serial` is the USB serial port. If multiple classes are initialized, then multiple serial ports can be used.

#### baudrate (int)
The baudrate of the serial port or stream. In a later version of the library, this will calculate the maximum DMX output rate. The library assumes that the serial port or stream is opened with this baudrate in `setup()`.

#### mode (int)
The type of device to emulate:

| value | description                                                  |
|-------|--------------------------------------------------------------|
| 0     | A standard ENTTEC-compatible device with one universe output |
| 1     | A DMXKing two-universe device                                |

Note that mode 0 only responds to ENTTEC label 6 and outputs to universe 0 in the callback. When mode is 1, label 6 outputs to both universe 0 and 1 (the callback is called twice) as per the DMXKing specification. With mode 1, label 100 outputs to only universe 0 and label 101 outputs to only universe 1.

#### callback (void)
A callback function to call when a DMX transmission is received. This function should have three parameters:

| parameter          | description                                                             |
|--------------------|-------------------------------------------------------------------------|
| int universe       | An integer starting at `0` with which universe received the DMX message |
| char buffer[512]   | The array of 512 DMX values starting at 0                               |

Example function that lights an LED with channel 1 on universe 0:
```cpp
void myDMXCallback(int universe, char buffer[512]) {
  for (int index=0; index < 512; index++) { // for each channel, universe starts at 0
    int channel = index + 1; // channel starts at 0, so index 0 is DMX channel 1 and index 511 is DMX channel 512
    int value = buffer[index]; // DMX value 0 to 255
    if (universe == 0 && channel == 1) analogWrite(LED_PIN, value); // LED on channel 1 on universe 0
  }
}
```

### DMXUSB.listen ()
A function of the class that causes the library to check for messages. This function is typically called at the top of `loop()`. Example:
```cpp
void loop() {
  myDMXUsb.listen();
}
```

## References
This library was built using the following sources:

 - [ENTTEC DMX USB Pro Widget API Specification 1.44](https://dol2kh495zr52.cloudfront.net/pdf/misc/dmx_usb_pro_api_spec.pdf)
 - [DMXKing ultraDMX Pro User Manual](https://www.pjrc.com/teensy/td_uart.html)
 - [Open Lighting Protocol USB Protocol Extensions Reference](https://wiki.openlighting.org/index.php/USB_Protocol_Extensions#Device_Manufacturer.2C_Label_.3D_77.2C_no_data)
 - [Open Lighting Project Mailing List Help Thread](https://groups.google.com/forum/#!topic/open-lighting/SIMMzwRcxPY)
 - [Basic Proof-of-Concept of ENTTEC device emulation](https://github.com/PaulStoffregen/Lighting_Controller/blob/master/electronics/CorePlay/CorePlay.ino)

## License
Please see the [LICENSE file](LICENSE)
