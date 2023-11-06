#!/bin/bash

if [[ $OPENSWITCHER_INSTALL_WITH_DPKG == 1 ]]; then
	programpath=/usr/bin/openswitcher
else
	programpath=/usr/local/bin/openswitcher
fi

sudoerspath=/etc/sudoers.d/openswitcher

if [ ! "$SUDO_USER" == "" ]; then
	OPENSWITCHERUSER="$SUDO_USER"
else
	OPENSWITCHERUSER="$USER"
fi

HOME_USER=$(eval echo "~$OPENSWITCHERUSER")

if [ "$XDG_CONFIG_HOME" == "" ]; then
	export XDG_CONFIG_HOME="$HOME_USER/.config"
fi

rule="
# Rule for correctly working OpenSwitcher programm 
$OPENSWITCHERUSER ALL = (root) NOPASSWD: $programpath
"

# Requested files, can be configured yourself
required_files=("$XDG_CONFIG_HOME/actkbd/actkbd.conf" "$HOME_USER/.xinitrc")

default_autostart_actkbd="sudo actkbd --daemon --config $XDG_CONFIG_HOME/actkbd/actkbd.conf  &"

# Activate OpenSwitcher using ctr+alt
default_combination="# Activate OpenSwitcher
# To read about how to customise read actkbd documentation
# To read where is keys see #include <linux/input-event-codes.h>
# 29=Ctr_L 56=Alt_L
29+56:::openswitcher.sh"

# Requested packages 
required_packages=("build-essential" "xserver-xorg" "libx11-dev" "libxkbcommon-x11-dev" "libxkbcommon-dev" "libicu-dev" "xsel")

distro=$(cat /etc/os-release | grep -w ID | cut -d'=' -f2)
