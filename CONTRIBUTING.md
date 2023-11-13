# Contributing

## Background

If you want support OpenSwitcher project you need know several things:

### Directories

1. Main programm is an openswitcher and it is compiled from the src/main.c
2. Directory src/ has sources for main openswitcher programm and others
3. Directory scripts/ has helping an instaling scripts for example input-device-info.sh
4. Directory doc/ has all documentation for openswitcher programm
5. Directory include/ has all include files for main openswitcher programm and others
6. Directory debian/ has files and subdirectories for build debian package(.deb)
7. Directory tests/ has all scripted and compiled tests
8. Directory actkbd/ is a submodule of `actkbd` programm
9. Directory xkb-switch/ is a submodule of `xkb-switch` programm

### Dependencies

#### For run

OpenSwitcher project work on this programs:

1. [xsel](https://github.com/kfish/xsel)
2. [actkbd](https://github.com/thkala/actkbd)
3. [xkb-switch](https://github.com/grwlf/xkb-switch)

#### For build

pkg-config modules

1. x11
2. xkbcommon
3. icu-uc
4. icu-io

On Debian
`sudo apt install build-essential xserver-xorg libx11-dev libxkbcommon-x11-dev libicu-dev xorg-dev`

## What is need ?

Check [TODO](TODO.md), test programm, read code, improve, write tests and docs

## Good luck :)
