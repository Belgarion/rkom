/* $Id: scr.c,v 1.1 2000/10/15 11:59:39 jens Exp $ */

#include <stdarg.h>
#include <curses.h>

#include "scr.h"

WINDOW *cmdwin;

void
scr_start(void)
{

	initscr();
	if (LINES < 24 || COLS < 80)
		scr_errx("screen is too small (%d,%d)", LINES, COLS);

	crmode();
	noecho();

	/* a single row at the bottom of the screen */
	cmdwin = newwin(1,0, LINES-1,0);
}

void
scr_cleanup(void)
{
	delwin(cmdwin);
	endwin();
}

void
scr_errx(const char *fmt, ...)
{
	va_list	ap;

	scr_cleanup();
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	exit(1);
}

void
scr_warnx(WINDOW *win, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vwprintw(win, fmt, ap);
	va_end(ap);
}

int
scr_read_prompt(WINDOW *win, const char *prompt, char *buf, int buf_len)
{
	mvwprintw(win, 0, 0, prompt);
	echo();
	wgetstr(win, buf);
	noecho();
	return 0;
}
