
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>

#include "rkom.h"

/*
 * Reads input from keyboard, and tries to understand it.
 */
int
kbd_input(fd)
{
	int s, d;
	char *buf, *dst;

	buf = get_input_string(fd, 1);

	if (buf == 0)
		return 1;
	dst = alloca(strlen(buf) + 1);
	s = d = 0; /* source/dest index in string */

	while (isspace(buf[s]))
		s++;

	if (buf[s] == 0) {
		strcpy(buf, prompt);
		dst = alloca(strlen(buf) + 1);
		s = 0;
	}

	/* Blank out everything inside parenthesis. */
	while (buf[s]) {
		if (buf[s] == '(') {
			while (buf[s] && buf[s] != ')') {
				buf[s] = ' ';
				s++;
			}
			buf[s] = ' ';
		}
		s++;
	}

	s = 0;
	while (isspace(buf[s]))
		s++;
	while (buf[s]) {
		if (buf[s] == ' ' && buf[s + 1] == ' ') {
			s++;
			continue;
		}
		dst[d] = buf[s];
		d++;s++;
	}
	dst[d] = 0;
	free(buf);
	while (isspace(dst[strlen(dst) - 1]))
		dst[strlen(dst) - 1] = 0;
	cmd_parse(dst);
	return 0;
}

char *
get_input_string(int fd, int brk)
{
	int i = 0, len;
	char *buf = 0;

	errno = 0;
	do {
		i++;
		buf = realloc(buf, 80 * i);
		len = read(fd, &buf[80 * (i - 1)], 80);
		if (len == 0 || errno) {
			errno = 0;
			if (brk) {
				free(buf);
				return 0;
			}
		}
	} while (len == 80);
	buf[80 * (i - 1) + len - 1] = 0;

	return buf;
}
