#!/bin/bash

if [[ $OPENSWITCHER_INSTALL_WITH_DPKG == 1 ]]; then
	source "/usr/lib/openswitcher/scripts/header.sh"
	source "/usr/lib/openswitcher/scripts/installfiles.sh"
else
	source "scripts/header.sh"
	source "scripts/installfiles.sh"
fi



# add rule to sudoers
if [ ! -f "$sudoerspath" ]; then
	echo "Добавляется правило для корректной работы OpenSwitcher в $sudoerspath:"
	echo "$rule"
	sudo touch $sudoerspath
	echo "$rule" | sudo tee $sudoerspath > /dev/null

	if ! sudo grep -q "$rule" "$sudoerspath"; then
		echo "Правило не добалено, завершение установки"
		exit 1
	else
		echo "Правило добалено"
	fi
	
else

	if ! sudo grep -q "$rule" "$sudoerspath"; then
		echo "Правило не добалено, попытка добавления правила"
		echo "$rule" | sudo tee $sudoerspath

		if ! sudo grep -q "$rule" "$sudoerspath"; then
			echo "Правило не добалено, завершение установки"
			exit 1
		else
			echo "Правило добалено"
		fi
	else
		echo "Правило добалено"
	fi
	
fi

if [[ $OPENSWITCHER_INSTALL_WITH_DPKG != 1 ]]; then
	# install xkb-switch
	echo "Устанока xkb-switch"
	if which cmake >/dev/null; then
		echo "cmake найден"
		echo "Установка xkb-switch"
		cd xkb-switch/
		mkdir -p build 
		cd build
		# cmake ..
		# make
		sudo make install
		sudo ldconfig
		cd ..
		cd ..
	else
		echo "cmake не найден"
		exit 1
	fi

	# install actkbd
	echo "Устанока actkbd"
	if sudo modprobe evdev >/dev/null; then
		cd actkbd/
		echo "evdev найден"
		# make
		sudo make install
		cd ../
	else
		echo "evdev не найден"
		exit 1
	fi
fi



sudo openswitcher --run

echo "Установка завершена."
