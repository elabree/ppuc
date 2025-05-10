# PPUC - Pinball Power-Up Controllers

The *Pinball Power-Up Controllers* are a set of hard- and software designed to repair and enhance the capabilities of
classic pinball machines of the 80s and 90s and to drive the hardware of home brew pinball machines.
The project is in ongoing development. Visit the [PPUC Page](http://ppuc.org) for further information.
This directory contains the PinMAME related parts, mainly the building blocks to emulate a pinball CPU that drives
PPUC I/O boards.

## Motivation

We want to enable people to be creative and to modernize old pinball machines using today's technology. Our goal is to
establish an open and affordable platform for that. Ideally people will publish their game-specific PPUs so others could
leverage and potentially improve them. We want to see a growing library of so-called *Pinball Power-Ups* (PPUs) and a
vital homebrew pinball community.

## Licences

The code in this directory and all sub-directories is licenced under GPLv3, except if a different license is mentioned
in a file's header or in a sub-directory. Be aware of the fact that your own enhancements of ppuc need to be licenced
under a compatible licence.

PPUC uses
* [libpinmame](https://github.com/vpinball/pinmame)
* [libdmdutil](https://github.com/vpinball/libdmdutil)
* [cargs](https://github.com/likle/cargs)
* [yaml-cpp](https://github.com/jbeder/yaml-cpp)
* [openal-soft](https://github.com/kcat/openal-soft/)
* [libppuc](https://github.com/PPUC/libppuc)

## Documentation

These components are still in an early development stage and the documentation will grow.

### Command Line Options

* -c path
    * path to config file
    * required
* -r rom name
    * rom to use, overwrites *rom* setting in config file
    * optional
* -s serial device
    * serial device path to use, overwrites *serialPort* setting in config file
    * optional
* -d
    * enable debug mode, overwrites *debug* setting in config file
    * optional
* -u
    * enable Serum colorization
    * optional
* -t VALUE
    * Serum timeout in milliseconds to ignore unknown frames
    * optional
* -p VALUE
    * Serum ignore number of unknown frames
    * optional
* -i
    * render display in console
    * optional
* -h
    * help


### Compiling

#### Windows (x64)

```shell
platforms/win/x64/build.sh
```

#### Windows (x86)

```shell
platforms/win/x86/build.sh
```

#### Linux (x64)
```shell
platforms/linux/x64/build.sh
```

##### Ubuntu 23.10 Example
```shell
sudo apt install git autoconf libtool libudev-dev libpipewire-0.3-dev libwayland-dev libdecor-0-dev liburing-dev libasound2-dev libpulse-dev libaudio-dev libjack-dev libsndio-dev libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev
git clone https://github.com/PPUC/ppuc.git
cd ppuc
platforms/linux/x64/build.sh
ppuc/ppuc-pinmame -c examples/t2.yml -n -i
```

#### Linux (aarch64)
```shell
platforms/linux/aarch64/build.sh
```

#### MacOS (arm64)
```shell
platforms/macos/arm64/build.sh
```

#### MacOS (x64)
```shell
platforms/macos/x64/build.sh
```
