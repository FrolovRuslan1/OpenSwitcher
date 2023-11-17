# OpenSwitcher
Utility for switching keyboard layout of currently selected text.

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)	
[![standard-readme compliant](https://img.shields.io/badge/readme%20style-standard-brightgreen.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

openswitcher - is a main program of OpenSwitcher project that takes selected text in standart input and transform it to sequence of keystrokes and emulates it.

## Table of Contents

- [Security](#security)
- [Background](#background)
- [Install](#install)
- [Uninstall](#uninstall)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)
<!-- - [API](#api) -->
## Security

### Sudoers rule

OpenSwitcher uses sudoers rule
```
# Rule for correctly working OpenSwitcher programm 
$USER ALL = (root) NOPASSWD: /usr/bin/openswitcher
```
in `/etc/sudoers.d/openswitcher` for correct operation because it works on `evdev` Linux module to avoid using X11.

## Background

### Programs that are used

1. `openswitcher` for more information see `openswitcher --help` or `man openswitcher`
2. `xkb-switch` for more information see `xkb-switch --help` or `man xkb-switch`
3. `actkbd` for more information see `sudo actkbd --help`
4. `xsel` for more information see `xsel --help` or `man xsel`

### Packages for build

#### Debian

`sudo apt install build-essential xserver-xorg libx11-dev libxkbcommon-x11-dev libicu-dev xorg-dev`

## Install

1. `git clone https://github.com/FrolovRuslan1/OpenSwitcher`
2. `cd OpenSwitcher`
3. **Read INSTALL**
4. `./configure`
5. `make`
6. `sudo make install OPENSWITCHER_LOCAL_INSTALL=1`

## Uninstall

1. `git clone https://github.com/FrolovRuslan1/OpenSwitcher`
2. `cd OpenSwitcher`
3. `./configure`
4. `sudo make uninstall OPENSWITCHER_LOCAL_INSTALL=1`

## Usage

### Run in a grafical environment

1. Select the text
2. Press default Ctr_L+Alt_L combination
3. Voila ! :)

### Run in a terminal

1. Select the text
2. Change keyboard layout
3. `xsel -o | sudo openswitcher --emulate --device "$(input-device-info.sh)"`


<!-- ## API

### Any optional sections -->

## Make .deb package

1. `git clone https://github.com/FrolovRuslan1/OpenSwitcher`
2. `cd OpenSwitcher`
3. `./configure`
4. `make deb`

## Docs generating

for Debian
`sudo apt install doxygen`

1. `git clone https://github.com/FrolovRuslan1/OpenSwitcher`
2. `cd OpenSwitcher`
3. `./configure`
4. `doxygen`
5. open with browser `doc/html/index.html`

## Contributing	

See [the contributing file](CONTRIBUTING.md)!

## License

GPLv3