/* $Id: conf.c,v 1.5 2000/10/15 21:39:15 jens Exp $ */

#include <sys/cdefs.h>
#include <sys/param.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <curses.h>

#include "conf.h"
#include "scr.h"
#include "keys.h"

#include <assert.h>

#define CTRL(x) ((int)(0xf&(x)))

static WINDOW *thrdwin, *statwin, *infowin, *textwin;
static int thrdwin_height = 10;
static int textwin_height;

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

enum {
	M_UNBOUND = KD_UNBOUND,
	M_NEXT_ENTRY,
	M_PREVIOUS_ENTRY,
	M_NEXT_PAGE,
	M_PREVIOUS_PAGE,
	M_NEXT_LINE,
	M_PREVIOUS_LINE,
	M_REFRESH,
	M_QUIT
};



static key_def_t kd_tab[] = {
/* keys to move in the thread window */
{ 'j',			M_NEXT_ENTRY,			"next-entry"			},
{ KEY_DOWN,		M_NEXT_ENTRY,			"next-entry"			},
{ 'k',			M_PREVIOUS_ENTRY,		"previous-entry"		},
{ KEY_UP,		M_PREVIOUS_ENTRY,		"previous-entry"		},

/* keys to move in the text window */
{ ' ',			M_NEXT_PAGE,			"next-page"				},
{ KEY_NPAGE,	M_NEXT_PAGE,			"next-page"				},
{ '-',			M_PREVIOUS_PAGE,		"previous-page"			},
{ KEY_PPAGE,	M_PREVIOUS_PAGE,		"previous-page"			},
{ '\n',			M_NEXT_LINE,			"next-line"				},
{ CTRL('h'),	M_PREVIOUS_LINE,		"previous-line"			},

{ CTRL('L'),	M_REFRESH,				"refresh-screen"		},
{ 'q',			M_QUIT,					"quit-conference-menu"	}
};


void
conference_menu(void)
{
	int			key, i, num_elem, kd_id;
	keybind_t	*kb;
	

	/*
	 * Window at the top of the screen that shows all the threads in
	 * the conference.
	 */
	thrdwin = newwin(thrdwin_height,0, 0,0);

	/* Window that show the status of the conference shown. */
	statwin = newwin(1,0, thrdwin_height,0);
	wattrset(statwin, A_REVERSE);

	textwin_height = LINES-3-thrdwin_height;
	/* Window in which the current text is shown in */
	textwin = newwin(textwin_height,0, thrdwin_height+1,0);

	/* Window which show some info of the current text */
	infowin = newwin(1,0, LINES-2,0);
	wattrset(infowin, A_REVERSE);

	keypad(stdscr, 1);

	kb = keybind_init();
	num_elem = sizeof(kd_tab) / sizeof(key_def_t);
	for (i = 0; i < num_elem; i++)
		keybind_add(kb, &kd_tab[i]);
	

	goto_msg_num(0);

	/* main loop if the conference menu */
	for ( ;; ) {
		/* hide cursor in something black */
		wmove(infowin, getmaxx(infowin), getmaxy(infowin));
		wrefresh(infowin);

		key = getch();
		werase(cmdwin);
		wrefresh(cmdwin);
		kd_id = keybind_lookup(kb, key);
		switch (kd_id) {
		case M_NEXT_ENTRY:
			goto_msg_num(at_msg + 1);
			break;
		case M_PREVIOUS_ENTRY:
			goto_msg_num(at_msg - 1);
			break;
		case M_NEXT_LINE:
			scroll_text(1);
			break;
		case M_NEXT_PAGE:
			scroll_text(textwin_height);
			break;
		case M_PREVIOUS_LINE:
			scroll_text(-1);
			break;
		case M_PREVIOUS_PAGE:
			scroll_text(-textwin_height);
			break;
		case M_REFRESH:
			wrefresh(stdscr);
			break;
		case M_QUIT:
			goto end_menu;
		case M_UNBOUND:
			beep();
			scr_warnx(cmdwin, "Unbound key %d", key);
			break;
		default:
			assert(0);
		}
	}

end_menu:
	keybind_free(kb);
	keypad(stdscr, 0);
	delwin(thrdwin);
	delwin(statwin);
	delwin(textwin);
	delwin(infowin);
}

void
conf_refresh(void)
{
	char	buf[81];

	/* update status window */
	werase(statwin);
	snprintf(buf, sizeof(buf), " CKom | %-30.30s | Msgs:%d New:%d Old:%d",
		art_get_conf_name(), art_count(), art_count(), 0);
	wprintw(statwin, "%-80.80s", buf);
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
	char	buf[81];
	char	**msg_lines;
	int		last_row, i, tilde_fill, num_lines;

	art = art_get_num(at_msg);

	wmove(infowin, 0,0);
	werase(infowin);
	snprintf(buf, sizeof(buf), "(%d): %-30.30s | %-30.30s",
		art->art_id, art->art_from, art->art_real_subj);
	wprintw(infowin, "%-80.80s", buf);
	wrefresh(infowin);

	werase(textwin);

	num_lines = 0;
	for (i = 0; art->art_header[i] != NULL; i++)
		num_lines++;
	num_lines++;
	for (i = 0; art->art_text[i] != NULL; i++)
		num_lines++;
	num_lines++;
	for (i = 0; art->art_footer[i] != NULL; i++)
		num_lines++;
	num_lines++;

	if ((msg_lines = alloca(sizeof(char *) * num_lines)) == NULL)
		scr_errx("alloca: %s", strerror(errno));
	num_lines = 0;
	for (i = 0; art->art_header[i] != NULL; i++)
		msg_lines[num_lines++] = art->art_header[i];
	msg_lines[num_lines++] = "";
	for (i = 0; art->art_text[i] != NULL; i++)
		msg_lines[num_lines++] = art->art_text[i];
	msg_lines[num_lines++] = "";
	for (i = 0; art->art_footer[i] != NULL; i++)
		msg_lines[num_lines++] = art->art_footer[i];
	msg_lines[num_lines++] = NULL;

	last_row = at_msg_top_row + textwin_height;
	tilde_fill = 0;
	for (i = at_msg_top_row; i < last_row; i++) {
		if (tilde_fill || msg_lines[i] == NULL) {
			tilde_fill = 1;
			wmove(textwin, i - at_msg_top_row, 0);
			wprintw(textwin, "~");
			continue;
		}
		wmove(textwin, i - at_msg_top_row, 0);
		wprintw(textwin, "%-75.75s", msg_lines[i]);
	}

	wrefresh(textwin);
}

void
conf_thrd_refresh(void)
{
	art_t	*art;
	char	timestr[10], buf[80];
	int		i, last_msg;

	werase(thrdwin);
	last_msg = MIN(thrdwin_height + top_msg, art_count());
	for (i = top_msg; i < last_msg; i++) {
		art = art_get_num(i);
		wmove(thrdwin, i - top_msg, 0);
		if (i == at_msg) {
			wattrset(thrdwin, A_REVERSE);
			wprintw(thrdwin, "->");
			wattrset(thrdwin, A_NORMAL);
		} else
			wprintw(thrdwin, "  ");
		strftime(timestr, sizeof(timestr), "%b %d", localtime(&art->art_time));
		snprintf(buf, sizeof(buf), "%4d %-6.6s %-20.20s (%4d) %s",
			i, timestr, art->art_from, art->art_no_of_lines, art->art_subj);
		wprintw(thrdwin, "%-78.78s", buf);
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
	art_t	*art;
	int		i, num_lines, row_num;

	row_num = at_msg_top_row + num_rows;
	if (row_num < 0) {
		if (at_msg_top_row == 0) {
			beep();
			scr_warnx(cmdwin, "Allready at top of message");
			return;
		} else
			row_num = 0;
	}

	num_lines = 0;
	art = art_get_num(at_msg);
	for (i = 0; art->art_text[i] != NULL; i++)
		num_lines++;
	for (i = 0; art->art_header[i] != NULL; i++)
		num_lines++;
	for (i = 0; art->art_footer[i] != NULL; i++)
		num_lines++;

	if (row_num >= num_lines) {
		beep();
		scr_warnx(cmdwin, "Allready at end of message");
		return;
	}

	at_msg_top_row = row_num;
	conf_text_refresh();
}

