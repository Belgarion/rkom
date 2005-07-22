#ifdef LINUX
#define _GNU_SOURCE 1
#include <string.h>
#elif __FreeBSD__ == 5
#include <string.h>
#endif

#include <sys/param.h>

#include <stdio.h>
#include <stdarg.h>
#include <strings.h>
#include <stdlib.h>

#include "rkom.h"
#include "rkomsupport.h"
#include "backend.h"

int outlines, discard;

static char *cvtstr[] = {
	"}å", "]Å", "{ä", "[Ä", "|ö", "\\Ö", 
};

#if defined(SOLARIS) || defined(SUNOS4) || defined(AIX)
/*
 * Simple and dumb implementation of vasprintf(): loop around and try
 * bigger and bigger buffer to print result in.
 */
int vsnprintf(char *str, size_t count, const char *fmt, va_list args);
static int
vasprintf(char **ret, const char *format, va_list ap)
{
	char *buf;
	int size, r;

	size = 128;
#ifdef SUNOS4
	buf = malloc(10);	/* Must have an initial value */
#else
	buf = NULL;
#endif
	do {
		size *= 2;
		buf = realloc(buf, size);
		r = vsnprintf(buf, size, format, ap);
	} while (r >= size);
	*ret = buf;
	return r;
}
#endif

static void
to_swascii(char *str)
{
        int len = strlen(str);
        int i, j;

        for (i = 0; i < len; i++) {
                for (j = 0; j < sizeof(cvtstr)/sizeof(char *); j++)
                        if (index(cvtstr[j], str[i]))
                                str[i] = cvtstr[j][0];
        }
}


void
rprintf(char const *fmt, ...)
{
	va_list ap;
	char *utstr, *c, *d;
	int ch;

	if (discard)
		return;

	va_start(ap, fmt);

	vasprintf(&utstr, fmt, ap);
	c = utstr;
	while ((d = index(c, '\n'))) {
		*d++ = 0;
		if (swascii)
			to_swascii(c);
		printf("%s\n", c);
		outlines += (strlen(c)/wcols);
		if (outlines++ >= (wrows - 1)) {
			printf("(Tryck retur eller 'q' för att hoppa ur)");
			fflush(stdout);
			outlines = 1;
			rkom_loop(POLL_NETWORK|POLL_KEYBOARD|POLL_RET_KBD);
			ch = getchar();
			if (ch == 'q') {
				while (getchar() != '\n')
					;
				discard = 1;
				printf("\r");
				return;
			}
			printf("\r             \r");
		}
		c = d;
	}
	if (strlen(c)) {
		if (swascii)
			to_swascii(c);
		printf("%s", c);
	}
	free(utstr);

	va_end(ap);
}
