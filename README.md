# OpenSwitcher
Utility for switching layout of currently selected text

[![license](https://img.shields.io/github/license/:user/:repo.svg)](LICENSE)
[![standard-readme compliant](https://img.shields.io/badge/readme%20style-standard-brightgreen.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

openswitcher - is a main program of OpenSwitcher project that takes selected text in standart input and transform it to sequence of keystrokes and emulates it.

## Table of Contents

- [Security](#security)
- [Background](#background)
- [Install](#install)
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

### Packages for build

#### Debian

`sudo apt install build-essential xserver-xorg libx11-dev libxkbcommon-x11-dev libicu-dev xorg-dev`

## Install

1. `git clone -b test https://github.com/FrolovRuslan1/OpenSwitcher`
2. `cd OpenSwitcher`
3. **Read INSTALL**
4. `./configure`
5. `make`
6. `sudo make install OPENSWITCHER_LOCAL_INSTALL=1`

## Uninstall

1. `git clone -b test https://github.com/FrolovRuslan1/OpenSwitcher`
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
2.  `xkb-switch -n && xsel | sudo openswitcher --device "$(input-device-info.sh)"`


<!-- ## API

### Any optional sections -->

## Make .deb package

1. `git clone -b test https://github.com/pulkras/OpenSwitcher`
2. `cd OpenSwitcher`
3. `./configure`
4. `make deb`

## Docs generating

1.  `sudo apt install doxygen`

## Contributing	

See [the contributing file](CONTRIBUTING.md)!

## License

[MIT © Richard McRichface.](../LICENSE)