
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>

#include "rkom.h"

char *
get_input_string(int fd, int brk)
{
	int i = 0, len;
	char *buf = 0;

	do {
		i++;
		buf = realloc(buf, 80 * i);
		len = read(fd, &buf[80 * (i - 1)], 80);
	} while (len == 80);
	buf[80 * (i - 1) + len - 1] = 0;

	return buf;
}
