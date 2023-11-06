#!/bin/bash

my_dir="$(dirname "$0")"
source "$my_dir/header.sh"

echo "Удаление $sudoerspath"
sudo rm "$sudoerspath"

echo "Удаление стандартной комбаниции клавиш из $XDG_CONFIG_HOME/actkbd/actkbd.conf."
input_file="$XDG_CONFIG_HOME/actkbd/actkbd.conf"
output_file="$XDG_CONFIG_HOME/actkbd/actkbd.conf.tmp"

sudo grep -v "$default_combination" "$input_file" | sudo tee "$output_file" && sudo rm "$input_file" && sudo mv "$output_file" "$input_file"

echo "Удаление стандарного автозапуска из $HOME_USER/.xinitrc."
grep -v "$default_autostart_actkbd" "$HOME_USER/.xinitrc" > "/tmp/.xinitrc.tmp" && mv -f "/tmp/.xinitrc.tmp" "$HOME_USER/.xinitrc"

if [[ $OPENSWITCHER_INSTALL_WITH_DPKG != 1 ]]; then
	echo "Удаление xkb-switch"
	sudo rm /usr/local/bin/xkb-switch
	sudo rm /usr/local/lib/libxkbswitch.so.2.0.1
	sudo rm /usr/local/lib/libxkbswitch.so.2
	sudo rm /usr/local/lib/libxkbswitch.so
	sudo rm /usr/local/share/man/man1/xkb-switch.1.gz
	cd xkb-switch
	make clean
	cd ..

	prefix="/usr/local"
	sbindir="$prefix/sbin"
	sysconfdir="/etc"

	echo "Удалние actkbd"
	sudo rm -f "$sbindir/actkbd"
	sudo rm -f "$sysconfdir/actkbd.conf"
	cd actkbd
	make clean
	cd ..

fi






echo "Отсановка actkbd"
killall actkbd
