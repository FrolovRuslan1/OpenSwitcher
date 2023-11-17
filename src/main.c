// If you read it you shold know that X11 is absolutely old shit please try support to develop new Window Meneger
/*!
	@file
	@brief This is main program of OpenSwitcher project.

	This is main program of OpenSwitcher project compiled to openswitcher
	It takes options and standart input selected text if it is; process text to another keyboard layout
	after that it emulates keys pressing to print processed text.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <getopt.h>


#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <X11/XKBlib.h>

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-x11.h>


#include <unicode/ustring.h>
#include <unicode/ucnv.h>
#include <unicode/ustdio.h>
#include <unicode/utf8.h>
#include <unicode/uchar.h>
#include <unicode/utypes.h>


#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <linux/keyboard.h>

#include <sys/epoll.h>

#include <config.h>


uint8_t *read_text_from_stdin();
KeySym *transform_stdin_to_KeySyms(uint8_t *text);
int send_KeySym(KeySym keysym);
int send_key(int fd, struct input_event *ev, int value, KeyCode keycode);
int create_event(struct input_event *ev, int type, int code, int value);
int write_event(int fd, const struct input_event *ev);
uint8_t* append_char_to_string(uint8_t* str, uint8_t c);
KeySym KeyCodeToKeySym(Display * display, KeyCode keycode, unsigned int event_mask);
int options_handler(int argc, char * const argv[]);
char *get_prefix(const char *program);
int message(const char *text);
int debug(const char *text);
int count_codepoints(char* utf8_string);

static int verbose_flag = 0;
static int debug_flag = 0;
static int input_flag = 0;
static int output_flag = 0;
static char *config_path = "~/.config/actkbd/actkbd.conf";
// eventX is a keyboard driver; if you want to setup it try sudo evtest
static char* inputDevice = "/dev/input/event0";

/*!
* @brief Main function
*
* This function takes options and input selected text and
* emulates key presses of text.
*
* @param argc[in] Count of arguments.
* @param argv[in] pointer to arguments array of strings.
* @return 0 if Ok and -1 if error
*/
int main(int argc, char * const argv[])
{
    if (options_handler(argc, argv) != 0)
	{
        debug("options_handler() error");
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
    event.data.fd = STDIN_FILENO;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event) == -1)
	{
        debug("epoll_ctl() error");
        return -1;
    }

    struct epoll_event events[1];
    int num_events;

    // Wait for input on stdin
    num_events = epoll_wait(epoll_fd, events, 1, 0);
    if (num_events == -1)
	{
        debug("epoll_wait() error");
        return -1;
    } else if (num_events > 0 || input_flag) 
	{
		if (num_events > 0)
		message("Using input whithout input_flag");
		
        uint8_t* text = read_text_from_stdin();
        if (text == NULL) 
		{
            debug("read_text_from_stdin() error");
            return -1;
        }

        KeySym *arr = transform_stdin_to_KeySyms(text);
        if (arr == NULL) 
		{
            debug("transform_stdin_to_KeySyms() error");
            return -1;
        }

		int length = count_codepoints(text);
		if (length < 0) 
		{
            debug("count_code_points() error");
            return -1;
        }

		// Delete previos text
		// if (send_KeySym(XK_Delete) != 0) 
		// {
		// 	debug("send_KeySym() error");
		// 	return -1;
		// }
        for (int i = 0; i < length; i++) 
		{
            if (send_KeySym(arr[i]) != 0) 
			{
                debug("send_KeySym() error");
                return -1;
            }
        }

        free(text);
        free(arr);
    } else 
	{
        message("No input data.");
    }

    close(epoll_fd);
    
    return 0;
}

/*!
* @brief read text from stdin
*
* This function reads text from the standart input
* and return this text.
*
* @return pointer to the text if Ok or NULL if error
*/
uint8_t *read_text_from_stdin()
{
	message("read from stdin");
    uint8_t ch;
    uint8_t *text = malloc(1*sizeof(uint8_t));

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
KeySym *transform_stdin_to_KeySyms(uint8_t *text)
{
	message("transform stdin to KeySyms");
    int32_t length =  strlen(text);
	if (length < 0) 
	{
		debug("strlen() error");
		return NULL;
	}
	int32_t size = count_code_points(text);
    

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
		
		if (verbose_flag == 1)
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
int send_KeySym(KeySym keysym)
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


    int fd = open(inputDevice, O_WRONLY);
    if (fd == -1)
	{
        debug("open() error");
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
        send_key(fd, &ev, 1, XKeysymToKeycode(display, XK_Shift_L));
        send_key(fd, &ev, 1, keycode);
        send_key(fd, &ev, 0, XKeysymToKeycode(display, XK_Shift_L));
        send_key(fd, &ev, 0, keycode);

    // if it is a sign and if it equals itself with shift pressed
    } else if(!u_isalpha(codepoint) && KeyCodeToKeySym(display, keycode, event_mask) == keysym)
    {
        // shift sign
        send_key(fd, &ev, 1, XKeysymToKeycode(display, XK_Shift_L));
        send_key(fd, &ev, 1, keycode);
        send_key(fd, &ev, 0, XKeysymToKeycode(display, XK_Shift_L));
        send_key(fd, &ev, 0, keycode);
    }else
    {
        // lowercase sign or letter
        send_key(fd, &ev, 1, keycode);
        send_key(fd, &ev, 0, keycode);
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
int send_key(int fd, struct input_event *ev, int value, KeyCode keycode)
{
	if (output_flag)
	printf("%u ", keycode-8);
	
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
* @brief append char to the string
*
* This function appends char to the string
* 
* @param str[in] pointer to the string
* @param c[in] char that will be appended
* @return pointer to the string if Ok or NULL if error
*/
uint8_t* append_char_to_string(uint8_t* str, uint8_t c)
{
    int length = strlen((char *)str);
	if (length < 0)
	{
		debug("strlen() error");
		return NULL;
	}
	
    uint8_t* newStr = realloc(str, (length + 2) * sizeof(uint8_t));
    if (newStr == NULL)
	{
        debug("realloc error");
        return NULL;
    }

    newStr[length] = c;
    newStr[length + 1] = '\0';

    return newStr;
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
* @brief process program options
*
* This function process program options
* 
* @param argc[in] Count of arguments.
* @param argv[in] pointer to arguments array of strings.
* @return 0 if Ok and -1 if error
*/
int options_handler(int argc, char * const argv[])
{
	int c;

  	while (1)
    {
      	static struct option long_options[] =
        {
			{"verbose", no_argument,       		&verbose_flag, 	1},
			{"debug",		no_argument,		&debug_flag,	1},
			{"config",  	required_argument, 	0, 				'c'},
			{"help",    	no_argument,       	0, 				'h'},
			{"run",     	no_argument,       	0, 				'r'},
			{"stop",     	no_argument,      	0, 				's'},
			{"print-config",no_argument, 		0, 				'p'},
			{"version",		no_argument,		0,				'v'},
			{"device",		required_argument,	0,				'd'},
			{"input",		no_argument,		0,				'i'},
			{"output",		no_argument,		0,				'o'},
			{0, 0, 0, 0}
        };
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "c:hrsptvd:io",
                       long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
        break;

      	switch (c)
        {
        case 0:
			/* If this option set a flag, do nothing else now. */
			if (long_options[option_index].flag != 0)
				break;
			printf ("option %s", long_options[option_index].name);
			if (optarg)
				printf (" with arg %s", optarg);
			printf ("\n");
			break;

        case 'c':
			config_path = optarg;	
			break;
		
		case 'h':
		{
			puts("Usage: openswitcher [options]");
			puts("Options:");
			puts("  -h, --help                     	Display this help message.");
			puts("  -c, --config <path_to_config>  	Specify path to actkbd config. Default is ~/.config/actkbd/actkbd.conf");
			puts("  -r, --run                      	Run key combination event loop.");
			puts("  -s, --stop                     	Stop key combination event loop.");
			puts("  -v, --version                  	Print program version.");
			puts("  -d, --device <path_to_device>  	Specify path to keyboard device. Default is /dev/input/event0");
			puts("  -i, --input                    	Enable standart input to transform KeySyms to input-event-codes.");
			puts("  -o, --output                   	Enable standart output to get transformed input-event-codes.");
			puts("  -p, --print                     Print actkbd config.");
			puts("      --verbose                   Enable verbose mode.");
			puts("      --debug                     Enable debug mode.");
			puts("");
			puts("See also:");
			puts("sudo actkbd --help");
			puts("man xkb-switch");
			puts("man openswitcher");
			
			break;	
		}
		
		case 'v':
			printf("%s\n", VERSION);
			break;
		
		case 'p':
		{
			message("print config from:");
			message(config_path);
			size_t command_length = strlen(config_path) + strlen("cat ") + 1;
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
			snprintf(command, command_length, "cat %s", config_path);

			if (system(command) != 0)
			{
				debug("system() error");
				free(command);
				return -1;
			}
			
			free(command);
			break;
		}

		case 'r':
		{
			if (system("pgrep actkbd >/dev/null") == 0) {
				message("Restart actkbd");

				if (system("sudo killall actkbd") != 0)
				{
					debug("system() error");
					return -1;
				}
				
				size_t command_length = strlen(config_path) + strlen("sudo actkbd -D -c ") + 1;
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
				snprintf(command, command_length, "sudo actkbd -D -c %s", config_path);

				if (system(command) != 0)
				{
					debug("system() error");
					free(command);
					return -1;
				}

				free(command);
			}else
			{
				message("Run actkbd");

				size_t command_length = strlen(config_path) + strlen("sudo actkbd -D -c ") + 1;
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
				snprintf(command, command_length, "sudo actkbd -D -c %s", config_path);

				if (system(command) != 0)
				{
					debug("system() error");
					free(command);
					return -1;
				}

				free(command);
			}

			break;
		}

		case 's':
		{
			message("Stop actkbd");
			system("sudo killall actkbd");
			break;
		}
		
		case 'd':
			inputDevice = optarg;
			break;

		case 'i':
			input_flag = 1;
			break;
			
		case 'o':
			output_flag = 1;
			break;

        case '?':
			/* getopt_long already printed an error message. */
			break;

        default:
          	abort ();
        }
    }

	/* Print any remaining command line arguments (not options). */
	// if (optind < argc)
    // {
	// 	printf ("non-option ARGV-elements: ");
	// 	while (optind < argc)
	// 		printf ("%s ", argv[optind++]);
	// 	putchar ('\n');
    // }

	return 0;
}

/*!
* @brief get prefix from the program path
*
* This function gets prefix from the program path
* example
* program ls 
* path /usr/bin/ls
* prefix=/usr/
* 
* @param program[in] program name
* @return pointer to the prefix if Ok or NULL if error
*/
char *get_prefix(const char *program)
{
    char *which = "which ";
    char *which_command = malloc(strlen(which) + strlen(program) + 1);
    
    strcpy(which_command, which);
    strcat(which_command, program);

	FILE* fd = popen(which_command, "r");
	if (fd == NULL) {
        printf("Ошибка при открытии потока для команды\n");
        return NULL;
    }

	char c;
    char *path = malloc(sizeof(char));
	path[0] = '\0';

	while ((c = getc(fd)) != EOF)
	path = append_char_to_string(path, c);

	char *prefix = malloc(sizeof(char));
	prefix[0] = '\0';

	for (int i = 0; path[i] != 'b'; i++)
	prefix = append_char_to_string(prefix, path[i]);

	free(which_command);
	free(path);
	pclose(fd);

	return prefix;
}

/*!
* @brief print verbose message
*
* This function prints message for verbose mode
* 
* @param text[in] pointer to the text
* @return 0 if Ok and -1 if error
*/
int message(const char *text)
{
	if (verbose_flag)
	{
		int ret = puts(text);
		if (ret == EOF)
		return -1;
	}
	
	return 0;
}

/*!
* @brief print debug message
*
* This function prints message for debug mode
* 
* @param text[in] pointer to the text
* @return 0 if Ok and -1 if error
*/
int debug(const char *text)
{
	if (debug_flag)
	perror(text);
	
	return 0;
}

/*!
* @brief count UChar32 codepoints in string
*
* This function count UChar32 codepoints in string
* 
* @param utf8_string[in] pointer to string in utf8 that will be counted
* @return count of codepoints if Ok or -1 if error
*/
int count_codepoints(char* utf8_string)
{
    UChar32 codepoint;
    int count = 0;
    int32_t offsetIndex = 0;
    int32_t length = strlen(utf8_string);
	if (length < 0)
	{
		debug("strlen() error");
		return -1;
	}
    
    while (offsetIndex < length)
	{
		U8_NEXT(utf8_string, offsetIndex, length, codepoint);
        if (codepoint < 0 )
		{
			debug("U8_NEXT() error");
            return -1;
        }
		count++;
	} 
    
    return count;
}