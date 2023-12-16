// If you read it you shold know that X11 is absolutely old shit please try support to develop new Window Maneger
/*!
	@file
	@brief This is the functional file of OpenSwitcher project.

	This file implements a variety of functionality such as 
	read_text_from_stdin(), transform_stdin_to_KeySyms(char *text) and add_actkbd_shortcut().
*/
#include "functional.h"


/*!
* @brief read text from stdin
*
* This function reads text from the standart input
* and return this text.
*
* @return pointer to the text if Ok or NULL if error
*/
char *read_text_from_stdin()
{
	message("read from stdin");
    char ch;
    char *text = malloc(1*sizeof(char));

    if (text == NULL) {
        debug("malloc error");
        return NULL;
    }

    text[0] = '\0';

    for (size_t i = 1; read(STDIN_FILENO, &ch, 1) > 0; i++)
	{
		text = append_char_to_string(text, ch);
		if (text == NULL)
		{
			debug("append_char_to_string() error");
			return NULL;
		}
	}	
	
	if (strlen(text) <= 0)
	{
		debug("strlen error");
		return NULL;
	}
	
    return text;
}

/*!
* @brief transform text to KeySym array
*
* This function transform text to KeySym array
* 
* @param text[in] pointer to the text that will be transformed
* @return pointer to KeySym array if Ok or NULL if error
*/
KeySym *transform_stdin_to_KeySyms(char *text)
{
	message("transform stdin to KeySyms");
    int32_t length =  strlen(text);
	if (length < 0) 
	{
		debug("strlen() error");
		return NULL;
	}
	int32_t size = count_codepoints(text);
    

    KeySym *arr = (KeySym *)malloc(sizeof(KeySym)*size);
	if (arr == NULL)
	{
        debug("malloc error");
        return NULL;
    }

    UChar32 codepoint;
	int32_t index = 0;
	int32_t offsetIndex = 0;

	while (offsetIndex < length)
	{
		U8_NEXT(text, offsetIndex, length, codepoint);
		if (codepoint < 0 )
		{
			debug("U8_NEXT() error");
			free(arr);
            return NULL;
        }
		xkb_keysym_t keysym = xkb_utf32_to_keysym((uint64_t)codepoint);
		if (keysym == XKB_KEY_NoSymbol)
		{
			debug("xkb_utf32_to_keysym() error");
			free(arr);
			return NULL;
		}
		
		if (openswitcher.options.verbose_flag == 1)
		{
			char buffer[100];
			
			if (xkb_keysym_get_name(keysym, buffer, 100) < 0)
			{
				debug("xkb_keysym_get_name error");
				free(arr);
				return NULL;
			}
			

			char* codestr = (char*)malloc(sizeof(char)*15);
			if (codestr == NULL)
			{
				debug("malloc error");
				free(arr);
				free(codestr);
				return NULL;
			}
			snprintf(codestr, 15, "U+%04X", codepoint);
        	printf("index: %d codepoint: %s name: %s \n", index, codestr, buffer);
			free(codestr);
		}

        // There are expected to be KeySym = xkb_keysym_t
        arr[index] = (KeySym)keysym;
		index++;
	}

    return arr;
}

/*!
* @brief emulate KeySym press
*
* This function emulate KeySym press on
* particular keyboard event driver
* 
* @param keysym[in] symbol that will be emulated
* @return 0 if Ok and -1 if error
*/
int write_KeySym(int fd, KeySym keysym)
{	
	message("send KeySym");
    Display* display = XOpenDisplay(NULL);
    if (display == NULL)
	{
		debug("XOpenDisplay() error");
		return -1;
	}   

    KeyCode keycode = XKeysymToKeycode(display, keysym);
	if (keycode == 0)
	{
		debug("XKeysymToKeycode() error");
		return -1;
	}   


    // ---------------------------------------------------
    char* keysymName = XKeysymToString(keysym);
	if (keysymName == NULL)
	{
		debug("XKeysymToString() error");
		return -1;
	}  

    // message(keysymName);

    unsigned int event_mask = ShiftMask | LockMask;

    struct input_event ev;
    UChar32 codepoint = xkb_keysym_to_utf32(keysym);
	if (codepoint == 0)
	{
		debug("xkb_keysym_to_utf32() error");
		return -1;
	}

    // if it is letter and if this letter is uppercase
    if (u_isalpha(codepoint) && xkb_keysym_to_upper(keysym) == keysym)
    {
        // uppercase letter
        write_key(fd, &ev, 1, XKeysymToKeycode(display, XK_Shift_L));
        write_key(fd, &ev, 1, keycode);
        write_key(fd, &ev, 0, XKeysymToKeycode(display, XK_Shift_L));
        write_key(fd, &ev, 0, keycode);

    // if it is a sign and if it equals itself with shift pressed
    } else if(!u_isalpha(codepoint) && KeyCodeToKeySym(display, keycode, event_mask) == keysym)
    {
        // shift sign
        write_key(fd, &ev, 1, XKeysymToKeycode(display, XK_Shift_L));
        write_key(fd, &ev, 1, keycode);
        write_key(fd, &ev, 0, XKeysymToKeycode(display, XK_Shift_L));
        write_key(fd, &ev, 0, keycode);
    }else
    {
        // lowercase sign or letter
        write_key(fd, &ev, 1, keycode);
        write_key(fd, &ev, 0, keycode);
    }
	
    close(fd);
    XCloseDisplay(display);

    return 0;
}

/*!
* @brief send KeyCode to event driver
*
* This function sends KeyCode to particular
* event driver
* Scancode=keycode-8 is written into input event driver, not the KeyCode !!! 
* 
* @param fd[in] event driver file descriptor
* @param ev[in] pointer to the input_event structure
* @param value[in] 0 for release, 1 for keypress and 2 for autorepeat.(for keyboard)
* @param keycode[in] keycode that will be emulated
* @return 0 if Ok and -1 if error
*/
int write_key(int fd, struct input_event *ev, int value, KeyCode keycode)
{
	if (openswitcher.options.output_flag)
	printf("%u ", keycode-8);
	
	if (openswitcher.options.emulate_flag)
	{
		// keycode differs from scancode by 8
		// key event
		if (create_event(ev, EV_KEY, keycode-8, value))
		{
			debug("create_event() error");
			return -1;
		}
		
		if (write_event(fd, ev))
		{
			debug("write_event() error");
			return -1;
		}

		if (create_event(ev, EV_SYN, SYN_REPORT, 0))
		{
			debug("create_event() error");
			return -1;
		}
		
		if (write_event(fd, ev))
		{
			debug("write_event() error");
			return -1;
		}
	}
	
    return 0;
}

/*!
* @brief write event to event driver
*
* This function writes event to particular
* event driver
* 
* @param fd[in] event driver file descriptor
* @param ev[in] pointer to the input_event structure
* @return 0 if Ok and -1 if error
*/
int write_event(int fd, const struct input_event *ev)
{
	int ret = write(fd, ev, sizeof(*ev));
	if (ret == -1 || (size_t)ret < sizeof(*ev))
	{
		debug("write() error");
		return -1;
	}
	return 0;
}

/*!
* @brief create input_event structere
*
* This function creates input_event structere
* Scancode=keycode-8 is written into input event driver, not the KeyCode !!! 
* 
* @param ev[out] pointer to the input_event structure
* @param type[in] type of event
* @param code[in] scancode=keycode-8
* @param value[in] 0 for release, 1 for keypress and 2 for autorepeat.(for keyboard)
* @return 0 if Ok and -1 if error
*/
int create_event(struct input_event *ev, int type, int code, int value)
{
	ev->time.tv_sec = 0;
	ev->time.tv_usec = 0;
	ev->type = type;
	ev->code = code;
	ev->value = value;
	return 0;
}

/*!
* @brief transform KeyCode to KeySym
*
* This function transforms KeyCode to KeySym
* 
* @param display[in] pointer to the X server display
* @param keycode[in] keycode that will be transformed
* @param event_mask[in] event_mask that will be applied
* @return KeySym if Ok or NoSymbol if error
*/
KeySym KeyCodeToKeySym(Display * display, KeyCode keycode, unsigned int event_mask)
{
    KeySym keysym = NoSymbol;

    //Get the map
    XkbDescPtr keyboard_map = XkbGetMap(display, XkbAllClientInfoMask, XkbUseCoreKbd);
    if (keyboard_map) {
        //What is diff between XkbKeyGroupInfo and XkbKeyNumGroups?
        unsigned char info = XkbKeyGroupInfo(keyboard_map, keycode);
        unsigned int num_groups = XkbKeyNumGroups(keyboard_map, keycode);

        //Get the group
        unsigned int group = 0x00;
        switch (XkbOutOfRangeGroupAction(info)) {
            case XkbRedirectIntoRange:
                /* If the RedirectIntoRange flag is set, the four least significant
                 * bits of the groups wrap control specify the index of a group to
                 * which all illegal groups correspond. If the specified group is
                 * also out of range, all illegal groups map to Group1.
                 */
                group = XkbOutOfRangeGroupInfo(info);
                if (group >= num_groups) {
                    group = 0;
                }
            break;

            case XkbClampIntoRange:
                /* If the ClampIntoRange flag is set, out-of-range groups correspond
                 * to the nearest legal group. Effective groups larger than the
                 * highest supported group are mapped to the highest supported group;
                 * effective groups less than Group1 are mapped to Group1 . For
                 * example, a key with two groups of symbols uses Group2 type and
                 * symbols if the global effective group is either Group3 or Group4.
                 */
                group = num_groups - 1;
            break;

            case XkbWrapIntoRange:
                /* If neither flag is set, group is wrapped into range using integer
                 * modulus. For example, a key with two groups of symbols for which
                 * groups wrap uses Group1 symbols if the global effective group is
                 * Group3 or Group2 symbols if the global effective group is Group4.
                 */
            default:
                if (num_groups != 0) {
                    group %= num_groups;
                }
            break;
        }

        XkbKeyTypePtr key_type = XkbKeyKeyType(keyboard_map, keycode, group);
        unsigned int active_mods = event_mask & key_type->mods.mask;

        int i, level = 0;
        for (i = 0; i < key_type->map_count; i++) {
            if (key_type->map[i].active && key_type->map[i].mods.mask == active_mods) {
                level = key_type->map[i].level;
            }
        }

        keysym = XkbKeySymEntry(keyboard_map, keycode, level, group);
        XkbFreeClientMap(keyboard_map, XkbAllClientInfoMask, true);
    }

    return keysym;
}

/*!
* @brief read event
*
* This function reads event from secified event device
*
* @param fd[in] file descriptor of event device
* @param ev[out] pointer to input_event structure that will be fill
* @return 0 if Ok or -1 if error
*/
int read_event(int fd, struct input_event *ev)
{
	int ret = read(fd, ev, sizeof(*ev));
	if (ret == -1 || (size_t)ret < sizeof(*ev))
	{
		debug("read() error");
		return -1;
	}
	return 0;
}

/*!
* @brief add actkbd shortcut
*
* This function adds actkbd shortcut to the actkbd config
*
* @return 0 if Ok or -1 if error
*/
int add_actkbd_shortcut()
{
	GArray *combination = g_array_new(false, false, sizeof(unsigned short)); 
	message("Wait device to be ready...");
	if (get_key_combination(combination))
	{
		debug("get_key_combination() error");
        return -1;
	}

	char *combinationstr = combination_to_str(combination);
	char *shortcut = concat(combinationstr, openswitcher.shortcut);

	size_t command_length = strlen(shortcut) + strlen(openswitcher.actkbd_config_path) + strlen("echo \"\n\n\" >> ") + 1;
	if (command_length <= 0)
	{
		debug("strlen() error");
		return -1;
	}

	char *command = malloc(command_length);
	if (command == NULL)
	{
		debug("malloc() error");
		return -1;
	}
	snprintf(command, command_length, "echo \"\n%s\n\" >> %s", shortcut, openswitcher.actkbd_config_path);

	if (system(command) != 0)
	{
		debug("system() error");
		free(command);
		return -1;
	}

	free(command);
	free(shortcut);
	free(combinationstr);
	g_array_free(combination, true);
	
	return 0;
}

/*!
* @brief get key combination
*
* This function gets key combination from user input in event device
*
* @param combination[in] pointer to array of keycodes
* @return 0 if Ok or -1 if error
*/
int get_key_combination(GArray *combination)
{
	// can be error when user press Enter 1 sec
	if (usleep(1000000))
	{
		debug("usleep() error");
        return -1;
	}
	message("Press key combination or press ESC to exit.");

	int fd = open(openswitcher.input_device, O_RDONLY);
	if (fd == -1)
	{
        debug("open() error");
        return -1;
    }

	int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
	{
        debug("epoll_create1() error");
        return -1;
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event))
	{
        debug("epoll_ctl() error");
        return -1;
    }

	struct input_event ev;
	struct epoll_event events[1];
	do
	{
		if (wait_for_event(fd, &ev, epoll_fd, events))
		{
			debug("wait_for_event() error");
			return -1;
		}
		if (ev.code == KEY_ESC)
		return 0;
		
		if (ev.type == EV_SYN || ev.type == EV_MSC || ev.value == 2)
		continue;

		if (ev.value == 1)
		{
			g_array_append_val(combination, ev.code);
		}
	} 
	while(ev.value == 1 || (ev.type == EV_SYN || ev.type == EV_MSC || ev.value == 2));

	g_array_sort(combination, CompareFunc);
	puts("");
	for (size_t i = 0; i < combination->len; i++)
	{
		if(openswitcher.options.verbose_flag)
		printf("%d ", g_array_index(combination, unsigned short, i));
	}
	puts("");

	close(epoll_fd);
	close(fd);
	return 0;
}

/*!
* @brief wait for event
*
* This function waits for event on event device
*
* @param fd[in] file descriptor of event device
* @param ev[out] pointer to input_event structure that will be fill
* @param epoll_fd[in] file descriptor of epoll
* @param combination[in] pointer to array of epoll events
* @return 0 if Ok or -1 if error
*/
int wait_for_event(int fd, struct input_event *ev, int epoll_fd, struct epoll_event events[1])
{
	// Wait for input 
	int num_events = epoll_wait(epoll_fd, events, 1, -1);
	if (num_events == -1)
	{
		debug("epoll_wait() error");
		return -1;
	} 
	else if (num_events > 0) 
	{
		if (read_event(fd, ev))
		{
			debug("read_event() error");
			return -1;
		}
	} else 
	{
		message("No input data.");
	}
    
	return 0;
}

/*!
* @brief compare function
*
* This function compares 2 elements of Array
*
* @param a[in] pointer to value 
* @param b[in] pointer to value 
* @return -1 if a < b; 0 a == b; 1 a > b. 
*/
gint CompareFunc(gconstpointer a, gconstpointer b)
{
	unsigned short A = *((unsigned short *)a);
	unsigned short B = *((unsigned short *)b);
	if (A > B)
	{
		return 1;
	}
	else if (A < B)
	{
		return -1;
	}

	return 0;
}

/*!
* @brief transform combination to string
*
* This function transforms combination to formated actkbd string
*
* @param combination[in] pointer to array of keycodes
* @return poiner to formated string if Ok or NULL if error
*/
char *combination_to_str(GArray *combination)
{
	char *str = malloc(1*sizeof(char));
    if (str == NULL) 
	{
        debug("malloc error");
        return NULL;
    }
    str[0] = '\0';

	for (size_t i = 0; i < combination->len; i++)
	{
		int length = snprintf(NULL, 0, "%d", g_array_index(combination, unsigned short, i));
		char* number = malloc(length + 1);
		if (number == NULL)
		{
			debug("malloc error");
			return NULL;
		}
		snprintf(number, length + 1, "%d", g_array_index(combination, unsigned short, i));
		
		
		if (i+1 >= combination->len)
		{
			concat(str, number);
			free(number);
			break;
		}
		concat(str, number);
		concat(str, "+");

		free(number);
	}

	return str;
}
