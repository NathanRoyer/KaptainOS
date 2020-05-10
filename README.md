## KaptainOS

An unfinished Operating System written in x86 ASM and C.

Captain:
* Synonym of Chief
* Responsible of a ship good management

OS:
* Acronym of Operating System

KaptainOS is an operating system, a program responsible of a computer good management.

## Features

An OS must:
* host programs
* bring an abstraction layer between software and hardware
* be able to differentiate users
* maintain a stable and secure environment for the program it hosts

KaptainOS, as of now, can only fulfil the first two requirements.

## TodoList

# What's done:
* Bootloader
* Program in 32 bits mode
* Being able to program in C
* Generic IO / Bus driver
* Text-mode VGA driver
* Interrupts management
* Keyboard driver
* RAM management


# Yet to do:
* Full blown VGA driver
* PCI driver
* Network card driver
* Ethernet driver
* Multitasking (easy to do)
* Users management
* Securing the system

## Projet Structure
```
KaptainOS – Binaries and tools
├── boot
│   Bootloader's code
├── kernel - kernel and RAM stuff
│   └── interrupts
│       Interrupts management
├── utils
│   bonus code like strings & mset
├── services - drivers and relationship with peripherals
│	├── hardware
│	│   drivers that only talk to the hardware
│	├── drivers
│	│   drivers that process data taken from the hardware
│	└── software
│		drivers that process data from other programs
└── programs
    User programs
```
# How to run

Follow these steps to be able to build KapOS:
* Boot a computer connected to the internet & with Debian Linux installed
* Install make: `sudo apt-get install build-essential`
* cd to the root folder of this project
* Install the toolset: `sudo make install_build_tools`

Thou are now ready to modify, compile and run KaptainOS.

To open all C files and headers in 2 separate windows:
`make edit`

To compile the OS and boot it into the Qemu emulator:
`make`

To only compile the OS:
`make compile`

To test KapOS on real hardware:
* Warning : the following will erase the content your USB drive.
* Once it is compiled, upload it on a USB drive with the win_dd.bat file (run as admin in windows) in the root of the project.
* Boot the computer from the USB drive from the BIOS options.

# Built-in programs

There are only two included programs:
* a console - type `help` to peek at its features
* a snake game

Feel free to explore their code, it's relatively simple.

# Built-in drivers doc (french)

Doc is located in services/doc.txt

## That's it for the moment
