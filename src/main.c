// If you read it you shold know that X11 is absolutely old shit please try support to develop new Window Maneger
/*!
	@file
	@brief This is the main file of OpenSwitcher project.

	This file implements core functions for OpenSwitcher project such as
	options_handler(), read_config() or setup()
	and has struct Openswitcher for maintains state of program.
*/
#include "main.h"


struct Openswitcher openswitcher = 
{
	.options = {
		.verbose_flag = 0,
		.debug_flag = 0,
		.emulate_flag = 0,
		.input_flag = 0,
		.output_flag = 0,
	},
	.name = PACKAGE,
	.version = VERSION,
	.config_path = NULL,
	.actkbd_config_path = NULL,
	// eventX is a keyboard driver; if you want to setup it try `sudo evtest`
	.input_device = NULL,
	.username = NULL,
	.home = NULL,
	.xdg_config_home = NULL,
	.shortcut = NULL,
	.display = NULL 
};

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
	if(setup() != 0)
	{
		debug("setup() error");
        return EXIT_FAILURE;
	}

    if (options_handler(argc, argv) != 0)
	{
        debug("options_handler() error");
        return EXIT_FAILURE;
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
	{
        debug("epoll_create1() error");
        return EXIT_FAILURE;
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = STDIN_FILENO;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event))
	{
        debug("epoll_ctl() error");
        return EXIT_FAILURE;
    }

    struct epoll_event events[1];

    // Wait for input on stdin
    int num_events = epoll_wait(epoll_fd, events, 1, 0);
    if (num_events == -1)
	{
        debug("epoll_wait() error");
        return EXIT_FAILURE;
    } 
	else if (num_events > 0 || openswitcher.options.input_flag) 
	{
		if (num_events > 0)
		message("Using input whithout input_flag");

		openswitcher.display = XOpenDisplay(NULL);
		if (openswitcher.display == NULL)
		{
			debug("XOpenDisplay() error");
			return EXIT_FAILURE;
		} 
		
        char* text = read_text_from_stdin();
        if (text == NULL) 
		{
            debug("read_text_from_stdin() error");
            return EXIT_FAILURE;
        }

        KeySym *arr = transform_stdin_to_KeySyms(text);
        if (arr == NULL) 
		{
            debug("transform_stdin_to_KeySyms() error");
            return EXIT_FAILURE;
        }

		int length = count_codepoints(text);
		if (length < 0) 
		{
            debug("count_code_points() error");
            return EXIT_FAILURE;
        }

		// Delete previos text
		// if (write_KeySym(XK_Delete) != 0) 
		// {
		// 	debug("write_KeySym() error");
		// 	return -1;
		// }
		int fd = open(openswitcher.input_device, O_WRONLY);
		printf("openswitcher.input_device: %s\n", openswitcher.input_device);
		if (fd == -1)
		{
			debug("open() error");
			return EXIT_FAILURE;
		}
		message("write_KeySyms"); 
        for (int i = 0; i < length; i++) 
		{
			// if (usleep(50000))
			// {
			// 	debug("usleep() error");
			// 	return -1;
			// }
            if (write_KeySym(fd, arr[i])) 
			{
                debug("write_KeySym() error");
                return EXIT_FAILURE ;
            }
        }

		close(fd);
        free(text);
        free(arr);
		XCloseDisplay(openswitcher.display);
    } else 
	{
        message("No input data.");
    }

    close(epoll_fd);
	config_destroy(&openswitcher.cfg);
    
    return EXIT_SUCCESS;
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
			{"verbose", 			no_argument,       	&openswitcher.options.verbose_flag, 	1},
			{"debug",				no_argument,		&openswitcher.options.debug_flag,	1},
			{"config",  			required_argument, 	0, 				'c'},
			{"config-actkbd",		required_argument, 	0, 				'k'},
			{"help",    			no_argument,       	0, 				'h'},
			{"run",     			no_argument,       	0, 				'r'},
			{"stop",     			no_argument,      	0, 				's'},
			{"print",				no_argument, 		0, 				'p'},
			{"print-actkbd",		no_argument, 		0, 				'a'},
			{"version",				no_argument,		0,				'v'},
			{"device",				required_argument,	0,				'd'},
			{"input",				no_argument,		0,				'i'},
			{"output",				no_argument,		0,				'o'},
			{"emulate",				no_argument,		0,				'e'},
			{"add-shortcut",		required_argument,	0,				'b'},
			{0, 0, 0, 0}
        };
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "c:k:hrspavd:ioeb:",
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
			openswitcher.config_path = optarg;	
			break;

		case 'k':
			openswitcher.actkbd_config_path = optarg;	
			break;
		
		case 'h':
		{
			puts("Usage: openswitcher [options]");
			puts("Options:");
			puts("  -h, --help                     	        Display this help message.");
			puts("  -c, --config <path_to_config>  	        Specify path to openswitcher config. Default is ~/.config/openswitcher/config.cfg");
			puts("  -k, --config-actkbd <path_to_config>  	Specify path to actkbd config. Default is ~/.config/actkbd/actkbd.conf");
			puts("  -r, --run                      	        Run key combination event loop.");
			puts("  -s, --stop                     	        Stop key combination event loop.");
			puts("  -v, --version                  	        Print program version.");
			puts("  -d, --device <path_to_device>  	        Specify path to keyboard device. Default is /dev/input/event0");
			puts("  -i, --input                    	        Enable standart input to transform KeySyms to input-event-codes.");
			puts("  -o, --output                   	        Enable standart output to get transformed input-event-codes.");
			puts("  -p, --print                             Print openswitcher config.");
			puts("  -a, --print-actkbd                      Print actkbd config.");
			puts("  -e, --emulate                           Enable emulation mode. It emulates key presses of KeySyms input.");
			puts("  -b, --add-shortcut                      Adding shortcut to actkbd config.");
			puts("      --verbose                           Enable verbose mode.");
			puts("      --debug                             Enable debug mode.");
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
			message(openswitcher.config_path);
			size_t command_length = strlen(openswitcher.config_path) + strlen("cat ") + 1;
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
			snprintf(command, command_length, "cat %s", openswitcher.config_path);

			if (system(command))
			{
				debug("system() error");
				free(command);
				return -1;
			}
			
			free(command);
			break;
		}

		case 'a':
		{
			message("print config from:");
			message(openswitcher.actkbd_config_path);
			size_t command_length = strlen(openswitcher.actkbd_config_path) + strlen("cat ") + 1;
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
			snprintf(command, command_length, "cat %s", openswitcher.actkbd_config_path);

			if (system(command))
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

				if (system("killall actkbd"))
				{
					debug("system() error");
					return -1;
				}
				
				size_t command_length = strlen(openswitcher.actkbd_config_path) + strlen("actkbd -D -c ") + 1;
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
				snprintf(command, command_length, "actkbd -D -c %s", openswitcher.actkbd_config_path);

				if (system(command))
				{
					debug("system() error");
					free(command);
					return -1;
				}

				free(command);
			}else
			{
				message("Run actkbd");

				size_t command_length = strlen(openswitcher.actkbd_config_path) + strlen("actkbd -D -c ") + 1;
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
				snprintf(command, command_length, "actkbd -D -c %s", openswitcher.actkbd_config_path);

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
			if(system("sudo killall actkbd"))
			{
				debug("system() error");
				return -1;
			}
			
			break;
		}
		
		case 'd':
			openswitcher.input_device = optarg;
			break;

		case 'e':
			openswitcher.options.emulate_flag = 1;
			break;

		case 'i':
			openswitcher.options.input_flag = 1;
			break;
			
		case 'o':
			openswitcher.options.output_flag = 1;
			break;
		
		case 'b':
		{
			message("Adding shortcut to actkbd config.");
			openswitcher.shortcut = optarg;
			if (add_actkbd_shortcut())
			{
				debug("add_actkbd_shortcut() error");
				return -1;
			}
			
		}

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
* @brief read openswitcher config
*
* This function reads openswitcher config
* 
* @return count of codepoints if Ok or -1 if error
*/
int read_config()
{
	message("read openswitcher config from:");
	message(openswitcher.config_path);
	
    config_setting_t *setting;
    const char *str;

    config_init(&openswitcher.cfg);
	
    if (!config_read_file(&openswitcher.cfg, (const char *)openswitcher.config_path))
	{
		debug("config_read_file() error");
		perror("openswitcher.config_path");	
		perror(openswitcher.config_path);
        fprintf(stderr, "%s:%d - %s\n",
                config_error_file(&openswitcher.cfg),
                config_error_line(&openswitcher.cfg),
                config_error_text(&openswitcher.cfg));
        config_destroy(&openswitcher.cfg);
        return -1;
    }

    setting = config_lookup(&openswitcher.cfg, "openswitcher");
	if (setting == NULL)
	{
		debug("config_lookup() error");
		return -1;
	}

	int ret;
    ret &= config_setting_lookup_string(setting, "name" , 				(const char **)&openswitcher.name);
	ret &= config_setting_lookup_string(setting, "version", 			(const char **)&openswitcher.version);
	ret &= config_setting_lookup_string(setting, "config_path", 		(const char **)&openswitcher.config_path);
	ret &= config_setting_lookup_string(setting, "actkbd_config_path", 	(const char **)&openswitcher.actkbd_config_path);
	ret &= config_setting_lookup_string(setting, "input_device", 		(const char **)&openswitcher.input_device);
	ret &= config_setting_lookup_string(setting, "username", 			(const char **)&openswitcher.username);
	ret &= config_setting_lookup_string(setting, "home", 				(const char **)&openswitcher.home);
	ret &= config_setting_lookup_string(setting, "xdg_config_home", 	(const char **)&openswitcher.xdg_config_home);
	if (ret)
	{
		debug("config_setting_lookup_string() error");
		return -1;
	}

	setting = config_lookup(&openswitcher.cfg, "openswitcher.options");
	if (setting == NULL)
	{
		debug("config_lookup() error");
		return -1;
	}

	ret &= config_setting_lookup_bool(setting, "debug_flag", 	&openswitcher.options.debug_flag);
    ret &= config_setting_lookup_bool(setting, "verbose_flag", 	&openswitcher.options.verbose_flag);
	ret &= config_setting_lookup_bool(setting, "emulate_flag", 	&openswitcher.options.emulate_flag);
	ret &= config_setting_lookup_bool(setting, "input_flag", 	&openswitcher.options.input_flag);
	ret &= config_setting_lookup_bool(setting, "output_flag", 	&openswitcher.options.output_flag);
	if (ret)
	{
		debug("config_setting_lookup_string() error");
		return -1;
	}

    return 0;
}

/*!
* @brief setup openswitcher
*
* This function setups openswitcher program
* 
* @return count of codepoints if Ok or -1 if error
*/
int setup()
{
	message("openswitcher setup...");
	uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);

    if (pw == NULL) 
	{
    	debug("getpwuid() error");
        return -1;
    }

	// setup default config path
	if (strcmp(pw->pw_name, "root"))
	{
		// for user
		message("USER:");
		message(pw->pw_name);
		openswitcher.username = pw->pw_name;
		openswitcher.home = pw->pw_dir;
		openswitcher.xdg_config_home = malloc(strlen(pw->pw_dir) + strlen("/.config") + 1);
		if (openswitcher.xdg_config_home == NULL)
		{
			debug("malloc() error");
			return -1;
		}
		sprintf(openswitcher.xdg_config_home, "%s/.config", pw->pw_dir);

		openswitcher.config_path = malloc(strlen(openswitcher.xdg_config_home) + strlen("/openswitcher/config.cfg") + 1);
		if (openswitcher.config_path == NULL)
		{
			debug("malloc() error");
			return -1;
		}
		sprintf(openswitcher.config_path, "%s/openswitcher/config.cfg", openswitcher.xdg_config_home);
		
	}
	else if(getenv("SUDO_USER") != NULL && strcmp(getenv("SUDO_USER"), "root") != 0)
	{
		// for sudo user
		char *sudo_user = getenv("SUDO_USER");
		message("SUDO_USER:");
		message(sudo_user);		
		openswitcher.username = sudo_user;
		openswitcher.home = malloc(strlen(sudo_user) + strlen("/home/") + 1);
		if (openswitcher.home == NULL)
		{
			debug("malloc() error");
			return -1;
		}
		
		sprintf(openswitcher.home, "/home/%s", sudo_user);

		openswitcher.xdg_config_home = malloc(strlen(openswitcher.home) + strlen("/.config") + 1);
		if (openswitcher.xdg_config_home == NULL)
		{
			debug("malloc() error");
			return -1;
		}
		sprintf(openswitcher.xdg_config_home, "%s/.config", openswitcher.home);

		openswitcher.config_path = malloc(strlen(openswitcher.xdg_config_home) + strlen("/openswitcher/config.cfg") + 1);
		if (openswitcher.config_path == NULL)
		{
			debug("malloc() error");
			return -1;
		}
		sprintf(openswitcher.config_path, "%s/openswitcher/config.cfg", openswitcher.xdg_config_home);
				
	}else
	{
		// for root
		message("USER:");
		message(pw->pw_name);
		openswitcher.username = pw->pw_name;
		openswitcher.home = pw->pw_dir;
		openswitcher.xdg_config_home = malloc(strlen(pw->pw_dir) + strlen("/.config") + 1);
		if (openswitcher.xdg_config_home == NULL)
		{
			debug("malloc() error");
			return -1;
		}
		sprintf(openswitcher.xdg_config_home, "%s/.config", pw->pw_dir);

		openswitcher.config_path = malloc(strlen(openswitcher.xdg_config_home) + strlen("/openswitcher/config.cfg") + 1);
		if (openswitcher.config_path == NULL)
		{
			debug("malloc() error");
			return -1;
		}
		sprintf(openswitcher.config_path, "%s/openswitcher/config.cfg", openswitcher.xdg_config_home);
	}
	
	if (read_config() != 0)
	{
		debug("read_config() error");
		return -1;
	}

	return 0;
}
