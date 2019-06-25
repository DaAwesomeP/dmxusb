DMXUSB
========
[![License](https://img.shields.io/github/license/DaAwesomeP/dmxusb.svg?style=flat-square)](https://github.com/DaAwesomeP/dmxusb/blob/master/LICENSE) [![GitHub release](https://img.shields.io/github/release/DaAwesomeP/dmxusb.svg?style=flat-square)](https://github.com/DaAwesomeP/dmxusb/releases/latest) [![Github Releases Downloads](https://img.shields.io/github/downloads/DaAwesomeP/dmxusb/latest/total.svg?style=flat-square)](https://github.com/DaAwesomeP/dmxusb/releases/latest) [![build status](https://img.shields.io/travis/DaAwesomeP/dmxusb.svg?maxAge=2592000&style=flat-square)](https://travis-ci.org/DaAwesomeP/dmxusb)
---
DMXUSB implements the ENTTEC DMX USB Pro Widget API Specification 1.44 on any serial port. DMXUSB can emulate a single DMX port/universe device like the DMXKing USB ultraDMX Micro, a two port/universe device like the DMXKing ultraDMX Pro, or an n-universe DMXUSB device. All devices are compatible with the ENTTEC standard. DMXUSB works with the Open Lighting Architecture (OLA) as a usbserial device.

## Installation
DMXUSB is regularly tested on the Arduino Mega 2560 and the PJRC Teensy 3.2. Compilation tests for many more boards are [completed via Continuous Integration](https://travis-ci.org/DaAwesomeP/dmxusb).

### Via Arduino IDE Library Manager
In the Arduino IDE, go to `Sketch > Tools > Include Library > Manage Libraries` and search and install the latest release version DMXUSB. **If you are not using a PJRC Teensy board, then you will also need to install the elapsedMillis library.**

### Via the ZIP archive
Download the latest release ZIP from [here](https://github.com/DaAwesomeP/dmxusb/releases/latest) or the latest testing release from [here](https://github.com/DaAwesomeP/dmxusb/archive/master.zip). Then go to `Sketch > Tools > Include Library > Add ZIP Library`. **If you are not using a PJRC Teensy board, then you will also need to download and install the [elapsedMillis library](https://github.com/pfeerick/elapsedMillis/releases/latest).**

## Usage
Currently, the library only receives DMX messages from a PC over USB. Please take a look at the [`Simple_Test` sketch](examples/Simple_Test/Simple_Test.ino) for a complete example.

### DMXUSB (serial, baudrate, mode, callback, out_universes)
The DMXUSB class initializes a new instance of the library. The `out_universes` argument is only required for mode 2. Example:
```cpp
DMXUSB myDMXUsb(Serial, 115200, 0, myDMXCallback);
```

#### serial (Stream)
Any Stream-based serial port. On my Arduino-like boards, `Serial` is the USB serial port. If multiple classes are initialized, then multiple serial ports can be used.

#### baudrate (int)
The baudrate of the serial port or stream. The library assumes that the serial port or stream is opened with this baudrate in `setup()`.

Note that one raw DMX port runs at 250 kbps (250,000 baud). It is recommended to use the maximum available reliable baudrate for your device. Set the `mode` setting below accordingly with consideration for the error rates of your device.

Also note that the USB serial on Teensy boards always operate at 12 mbps regardless of setting. You should always set the baudrate to 12,000,000 baud for Teensy devices.

#### mode (int)
The type of device to emulate:

| value | description                                                                                                                                                                                        |
|-------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 0     | A standard ENTTEC-compatible device with one universe output that emulates the UltraDMX Micro (ESTA ID: 0x6A6B, Device ID: 0x3)                                                                    |
| 1     | A DMXKing two-universe device that emulates the UltraDMX Pro (0x6A6B, Device ID: 0x2)                                                                                                              |
| 2     | A DMXUSB n-universe device similar to the DMXKing two-universe device (requires the `out_universes` argument) **that is in development and not yet supported by OLA** (0x7FF7, Device ID: 0x32)    |

The following table shows which universes receive callbacks when different commands are sent to DMXUSB. When label 6 is received in all modes, the callback will be called multiple times and output to all universes as per the DMXKing specification.

| mode | label (command type) | universe 0  | univserse 1 | universes 2 through n |
|------|----------------------|-------------|-------------|------------------------|
| 0    | 6                    | Yes         | No          | No                     |
| 0    | 100                  | No          | No          | No                     |
| 0    | 101                  | No          | No          | No                     |
| 0    | 102 through n        | No          | No          | No                     |
| 1    | 6                    | Yes         | Yes         | No                     |
| 1    | 100                  | Yes         | No          | No                     |
| 1    | 101                  | No          | Yes         | No                     |
| 1    | 102 through n        | No          | No          | No                     |
| 2    | 6                    | Yes         | Yes         | Yes                    |
| 2    | 100                  | Yes         | No          | No                     |
| 2    | 101                  | No          | Yes         | No                     |
| 2    | 102 through n        | No          | No          | Yes                    |

Note that mode 0 only responds to ENTTEC label 6 and outputs to universe 0 in the callback. When mode is 1 or 2, label 6 outputs to either universe 0 and 1 for mode 1 and universes 0 through n for mode 2 (the callback is called multiple times). This mimics the DMXKing specification. With mode 1, label 100 outputs to only universe 0 and label 101 outputs to only universe 1.

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

#### out_universes (int)
The number of output universes. This parameter is required only when the mode is 2. **The number of output universes defaults to 0 if mode 2 is used and this parameter is not set.** This argument is ignored for modes 0 and 1.

The maximum number of theoretical universes is found with the following equation:
```
(maximum universes) = [(device baud rate) / (250,000 baud)] - 1
```
You must subtract one as seen above because there are a few extra bytes in every DMX packet when sent over USB with this library. This equation does not take into account CPU time and load or how long it takes to process the data or run the rest of your program. **With this math, the Teensy can theoretically achieve 47 universes!**

### DMXUSB.listen ()
A function of the class that causes the library to check for messages. This function is typically called at the top of `loop()`. Example:
```cpp
void loop() {
  myDMXUsb.listen();
}
```

## DMXUSB Device Specification (mode 2)
The DMXUSB Device extends the DMXKing UltraDMX Pro specification in the following ways:

### Labels 100 through 100 + n
Label 100 will send to universe 0, label 101 will send to universe 1, label 102 will send to universe 2, etc. Sending to more universes than are configured in `out_universes` or are responded with the Extended Parameters message is simply ignored.

### Label 53 Extended Parameters
Sending an empty message with label 53 will respond with the label 53 Extended Parameters Message with the following data:

1. Number of output universes (one byte)
2. Number of input universes (one byte)

More data may be added to this packet in the future.

## References
This library was built using the following sources:

 - [ENTTEC DMX USB Pro Widget API Specification 1.44](https://dol2kh495zr52.cloudfront.net/pdf/misc/dmx_usb_pro_api_spec.pdf)
 - [DMXKing ultraDMX Pro User Manual](https://www.pjrc.com/teensy/td_uart.html)
 - [Open Lighting Protocol USB Protocol Extensions Reference](https://wiki.openlighting.org/index.php/USB_Protocol_Extensions#Device_Manufacturer.2C_Label_.3D_77.2C_no_data)
 - [Open Lighting Project Mailing List Help Thread](https://groups.google.com/forum/#!topic/open-lighting/SIMMzwRcxPY)
 - [Basic Proof-of-Concept of ENTTEC device emulation](https://github.com/PaulStoffregen/Lighting_Controller/blob/master/electronics/CorePlay/CorePlay.ino)

## License
Please see the [LICENSE file](LICENSE)
