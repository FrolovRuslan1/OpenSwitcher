#!/bin/bash

programpath=/usr/local/bin/openswitcher
sudoerspath=/etc/sudoers.d/openswitcher
if [ ! "$SUDO_USER" == "" ]; then
	USER="$SUDO_USER"
fi
rule="
# Rule for correctly working OpenSwitcher programm 
$USER ALL = (root) NOPASSWD: $programpath
"
# sudoers="/etc/sudoers"


# if [ ! -f "$programpath" ]; then
#     echo "Файл $programpath не найден."
#     echo ""
#     if [ -f "./openswitcher" ]; then
#         echo "Копирование программы в /usr/bin/"
#         echo ""
#         sudo cp ./openswitcher $programpath
#         echo "Программа скопированна"
#     else
#         echo "Программа не скомпилированна в данной дериктории, установка не возможна"
#         exit 1
#     fi
# else
#     echo "Файл $programpath найден."
#     echo ""
# fi

# echo "Пожалуйста добавьте правило для работы OpenSwitcher в ручную, для вашей безопасности"
# echo "скопируйте и вставьте его при помощи комбинаций ctr+shift+c ctr+shift+v"
# echo "Правило:"
# echo "$rule"

# read -r -p "Хотите добавить правило для работы OpenSwitcher в файл /etc/sudoers? (y/n): " answer
# if [ "$answer" = "y" ] || [ "$answer" = "Y" ]; then
#     sudo visudo
#     if ! sudo grep -q "$rule" "$sudoers"; then
#         echo "Правило не добалено, завершение установки"
#         exit 1
#     else
#         echo "Правило добалено"
#     fi
# else
#     echo "Правило для работы OpenSwitcher не дабавлено, завершение установки"
#     exit 1
# fi


if [ ! -f "$sudoerspath" ]; then
	echo "Добавляется правило для корректной работы OPenSwitcher в $sudoerspath:"
	echo ""
	echo "$rule"
	sudo touch $sudoerspath
	echo "$rule" | sudo tee $sudoerspath

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

echo ""
echo "Добавте вручную команду привязанную к комбинации клавиш, которой вам будет удобно"
echo "Команда:"
echo "sh -c \"/usr/bin/xsel --primary -o | sudo /usr/bin/openswitcher\""
echo ""
echo "Используйте scripts/checkpkgs.sh для проверки и установки нужных пакетов"
