# PPUC - Build instructions for dummies

If you aren't an experienced programmer, the following instructions may help to compile PPUC.

## Install a Linux virtual machine

This is not needed in case you already have a computer running Linux:

1. Install [VirtualBox](https://www.virtualbox.org/wiki/Downloads)
2. Install [Ubuntu](https://ubuntu.com/tutorials/how-to-run-ubuntu-desktop-on-a-virtual-machine-using-virtualbox#1-overview) in a virtual machine.
3. Check that the [VirtualBox Guest Additions](https://itsfoss.com/virtualbox-guest-additions-ubuntu/) have been installed.

## Install the build environment (compiler)

Press CTRL+ALT+T to bring up a Terminal window (or click the terminal icon in the sidebar).

1st use Advanced Packaging Tool (APT) to update all packages
```shell
sudo apt-get update
sudo apt-get upgrade
sudo apt-get autoremove
```

2nd Install the packages required to build PPUC
```shell
sudo apt install curl
sudo apt-get install build-essential
sudo apt install cmake
sudo apt install zlib1g zlib1g-dev
```

### Compiling

Follow the instructions under **Compiling**, Ubuntu 23.10 Example
