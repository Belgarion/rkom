/* $Id: conf.c,v 1.2 2000/10/15 14:18:34 jens Exp $ */

#include <sys/cdefs.h>
#include <sys/param.h>
#include <curses.h>

#include "conf.h"
#include "scr.h"

#include <assert.h>

static WINDOW *thrdwin, *statwin, *infowin, *textwin;
static int thrdwin_height = 10;

/* the article shown at the top of the thrdwin window */
static int		top_msg = 0;

/* indentifes the current article */
static int		at_msg = 0;
static int		at_msg_top_row = 0;


static void conf_refresh(void);
static void conf_text_refresh(void);
static void conf_thrd_refresh(void);

static void goto_msg_num(int);

static void scroll_text(int);

void
conference_menu(void)
{
	int		key;

	/*
	 * Window at the top of the screen that shows all the threads in
	 * the conference.
	 */
	thrdwin = newwin(thrdwin_height,0, 0,0);

	/* Window that show the status of the conference shown. */
	statwin = newwin(1,0, thrdwin_height+1,0);

	/* Window in which the current text is shown in */
	textwin = newwin(LINES-3-thrdwin_height,0, thrdwin_height+2,0);

	/* Window which show some info of the current text */
	infowin = newwin(1,0, LINES-2,0);

	keypad(stdscr, 1);

	goto_msg_num(0);

	/* main loop if the conference menu */
	for ( ;; ) {
		key = getch();
		switch (key) {
		case 'j':
			goto_msg_num(at_msg + 1);
			break;
		case 'k':
			goto_msg_num(at_msg - 1);
			break;
		case '\n':
			scroll_text(1);
			break;
		case KEY_BACKSPACE:
			scroll_text(-1);
			break;
		case 'q':
			goto end_menu;
		default:
			beep();
			scr_warnx(cmdwin, "Unbound key '%c'", (char)key);
		}
	}

end_menu:
	keypad(stdscr, 0);
	delwin(thrdwin);
	delwin(statwin);
	delwin(textwin);
	delwin(infowin);
}

void
conf_refresh(void)
{
	/* update status window */
	werase(statwin);
	wprintw(statwin, " CKom| %-30.30s |Msgs:%d New:%d Old:%d",
		art_get_conf_name(), art_count(), art_count(), 0);
	wrefresh(statwin);

	/* update thread window */
	conf_thrd_refresh();
	
	/* update command window */
	werase(cmdwin);
	wrefresh(cmdwin);

	/* update text window */
	conf_text_refresh();
}

void
conf_text_refresh(void)
{
	art_t	*art;

	art = art_get_num(at_msg);

	wmove(infowin, 0,0);
	werase(infowin);
	wprintw(infowin, "  %d/%d: %-30.30s | %-30.30s",
		at_msg , art_count(), art->art_from, art->art_real_subj);
	wrefresh(infowin);

	werase(textwin);
	wprintw(textwin, "hej hopp ");
	wrefresh(textwin);
}

void
conf_thrd_refresh(void)
{
	art_t	*art;
	int		i, last_msg;

	werase(thrdwin);
	last_msg = MIN(thrdwin_height + top_msg, art_count());
	for (i = top_msg; i < last_msg; i++) {
		art = art_get_num(i);
		wmove(thrdwin, i - top_msg, 0);
		if (i == at_msg)
			wprintw(thrdwin, "->");
		else
			wprintw(thrdwin, "  ");
		wprintw(thrdwin, "%4d ", i);
		wprintw(thrdwin, "%-20.20s (%4d) %-30.30s",
			art->art_from, art->art_no_of_lines, art->art_subj);
	}
	wrefresh(thrdwin);
}

static void
goto_msg_num(int msg_num)
{
	if (msg_num < 0) {
		beep();
		scr_warnx(cmdwin, "Allready at top message");
		return;
	}

	if (msg_num >= art_count()) {
		beep();
		scr_warnx(cmdwin, "Cannot go to message number %d", msg_num);
		return;
	}

	at_msg_top_row = 0;

	if (msg_num < top_msg) {
		top_msg = msg_num;
		at_msg = msg_num;
		conf_refresh();
		return;
	}

	if (msg_num - top_msg + 1> thrdwin_height) {
		top_msg = msg_num - thrdwin_height + 1;
		at_msg = msg_num;
		conf_refresh();
		return;
	}

	at_msg = msg_num;
	conf_refresh();
}

static void
scroll_text(int num_rows)
{
	conf_text_refresh();
}
