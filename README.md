<!-- 
SPDX-FileCopyrightText: 2022 Sidings Media <contact@sidingsmedia.com>
SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
SPDX-License-Identifier: MIT
-->

# Network Clock

A small 7 segment display clock that uses NTP servers to synchronise
it's time all build around the ESP8266 

## Building

### Setting up your environment

#### Toolchain and SDK

Before you can build this project. You will first need to have setup the
toolchain required to compile code for the ESP8266. Instructions on how
to do this can be found in the documentation for the
[ESP8266 RTOS SDK](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/index.html#setup-toolchain).

After you have configured the toolchain, you will then need to download
the ESP8266 RTOS SDK made by Espressif. Instructions for how to do this
can be found in the [Get
ESP8266_RTOS_SDK](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/index.html#get-esp8266-rtos-sdk)
section of the SDK documentation.

#### Environment variables

In order to successfully flash the compiled binary to the ESP8266, you
may need to set two environment variables, `ESPPORT` which is the path
to the device the ESP8266 is listening on, for example `/dev/ttyUSB0`,
and `ESPBAUD`. `ESPBAUD` should normally be set to `115200` for
programming.

#### Other requirements

This project also depends upon CMake and Make. These should be installed
on your system.

### Building

#### Configuring the build system

First create the build directory and change into it.

```
mkdir build
cd build
```

Now configure the build system


```
cmake ..
```

#### Compile firmware and upload

First we compile the firmware.

```
make
```

And then we can flash it to the board.

```
make flash
```

The firmware should now be present on the board.

## Debugging

Debug statments are output on UART by the SDK. To view these, simply use
your favourite serial port terminal to connect to your ESP8266 at a baud
rate of `74880`. Because of this weird baudrate, I use
[tio](https://github.com/tio/tio) to connect as it supports setting non
standard baud rates, unlike some other software.

## Licence
This repo uses the [REUSE](https://reuse.software) standard in order to
communicate the correct licence for the file. For those unfamiliar with
the standard the licence for each file can be found in one of three
places. The licence will either be in a comment block at the top of the
file, in a `.license` file with the same name as the file, or in the
dep5 file located in the `.reuse` directory. If you are unsure of the
licencing terms please contact
[mnickson@sidingsmedia.com](mailto:mnickson@sidingsmedia.com).
All files committed to this repo must contain valid licencing
information or the pull request can not be accepted.
