// If you read it you shold know that X11 is absolutely old shit please try support to develop new Window Maneger
/*!
	@file
	@brief This is the utilities file of OpenSwitcher project.

	This file implements a variety of utilities for OpenSwitcher project such as
	message(), debug(), get_prefix(), append_char_to_string(), count_codepoints() and concat().
*/
#include "utilities.h"


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
	if (openswitcher.options.verbose_flag)
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
	if (openswitcher.options.debug_flag)
	perror(text);
	
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
char* append_char_to_string(char* str, char c)
{
    int length = strlen((char *)str);
	if (length < 0)
	{
		debug("strlen() error");
		return NULL;
	}
	
    char* newStr = realloc(str, (length + 2) * sizeof(char));
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
* @brief count UChar32 codepoints in string
*
* This function counts UChar32 codepoints in string
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

/*!
* @brief concatinate strings
*
* This function concatinates 2 strings
*
* @param str1[in] pointer to string
* @param str2[in] pointer to string
* @return poiner concatinated string if Ok or NULL if error
*/
char *concat(char *str1, char *str2)
{
    char *dest = NULL;
    size_t str1_length, str2_length;

    if(str1 && str2)
    {
        dest = malloc((str1_length = strlen(str1)) + (str2_length = strlen(str2)) + 1);
        if(dest)
        {
            memcpy(dest, str1, str1_length);
            memcpy(dest + str1_length, str2, str2_length);
        }
    }
    return dest;
}

