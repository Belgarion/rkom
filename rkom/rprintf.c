
#include <sys/param.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "rkom.h"

int outlines, discard;

static char *cvtstr[] = {
	"}å", "]Å", "{ä", "[Ä", "|ö", "\\Ö", 
};

static void
chrconvert(char *str)
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
			chrconvert(c);
		printf("%s\n", c);
		outlines += (strlen(c)/wcols);
		if (outlines++ >= (wrows - 1)) {
			printf("(Tryck retur)");
			outlines = 1;
			ch = getchar();
			if (ch == 'q') {
				while (getchar() != '\n')
					;
				discard = 1;
				printf("\r");
				return;
			}
		}
		c = d;
	}
	if (strlen(c)) {
		if (swascii)
			chrconvert(c);
		printf("%s", c);
	}
	free(utstr);

	va_end(ap);
}
