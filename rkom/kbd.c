
#include <sys/ioctl.h>

#include <termcap.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <err.h>
#include <unistd.h>

#include <strings.h>
#include <histedit.h>
#include <string.h>

#include "rkom.h"

static struct termios term;
static struct winsize ws;
static char caps[1024], bp[1024];
static char *cm;

void
tcapinit()
{
	char *ttype, *cap = caps;

	if ((ttype = getenv("TERM")) == 0)
		ttype = "vt100";

	if (tgetent(bp, ttype) != 1)
		err(1, "tgetent");

	cm = tgetstr("cm", &cap);

	ioctl(1, TIOCGWINSZ, &ws);

	tcgetattr(0, &term);
	term.c_lflag &= ~(ICANON|ECHO);
	term.c_cc[VMIN]=1;
	term.c_cc[VTIME]=0;
	tcsetattr(0, TCSAFLUSH, &term);
}

#define	READFAILED	-1
/*
 * Reads in a line from keyboard to the string buf of len maxlen and
 * at position off.
 * Returns
 */
static int
readin(char *buf, int off, int maxlen, int startoff)
{
	char c;
	int coff = off;

	bzero(&buf[off], maxlen - off);

	while ((read(0, &c, 1)) > 0) {
		write(1, &c, 1);
		if (c == '\n')
			return coff;
		buf[coff++] = c;
		if (coff == maxlen)
			return coff;
	}
	return READFAILED;
}

char *
getstr(char *m)
{               
	int rv, len = 0;
	char *buf = 0;

	write(1, m, strlen(m));
#define	CHUNK 80
	do {
		len += CHUNK;
		buf = realloc(buf, len);
		rv = readin(buf, len-CHUNK, len);
	} while (rv == CHUNK);
	return buf;
}

#if 0
static char *msg;

static char *
prompt_fun(EditLine *el)
{	
	return msg;
}	
 
char *
getstr(char *m)
{
	EditLine *el;
	char *ret;
	int len;  
	
	msg = m;
#if !defined(__FreeBSD__)
	el = el_init("rkom", stdin, stdout, stderr);
#else
	el = el_init("rkom", stdin, stdout);
#endif
	el_set(el, EL_EDITOR, "emacs");
	el_set(el, EL_PROMPT, prompt_fun);
	ret = strdup(el_gets(el, &len));
	ret[len - 1] = 0; /* Forget \n */
	el_end(el);
	return ret;
}
#endif
