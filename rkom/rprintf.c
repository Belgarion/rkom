
#include <sys/param.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "rkom.h"

int outlines;

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

	va_start(ap, fmt);

	vasprintf(&utstr, fmt, ap);
	c = utstr;
	while ((d = index(c, '\n'))) {
		*d++ = 0;
		if (swascii)
			chrconvert(c);
		printf("%s\n", c);
		if (outlines++ >= wrows) {
			printf("(Tryck retur)");
			getchar();
			outlines = 0;
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
