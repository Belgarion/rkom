/* $Id: conf.c,v 1.1 2000/10/15 11:59:39 jens Exp $ */

#include <sys/cdefs.h>
#include <curses.h>


#include "conf.h"
#include "scr.h"

#include <assert.h>

static WINDOW *thrdwin, *statwin, *infowin, *textwin;
static int thrdwin_height = 10;

/* the article shown at the top of the thrdwin window */
static int		top_text;
static artl_t	*top_al_pos;

/* indentifes the current article */
static int		at_text;
static art_t	*at_art;
static artl_t	*at_al_pos;


static void conf_refresh(conft_t *ct);
static void conf_text_refresh(conft_t *cf);
static void conf_thrd_refresh(conft_t *ct);

void
conference_menu(conft_t *ct)
{
	top_text = 0;
	top_al_pos = NULL;
	cl_art_walk(ct->ct_al, &top_al_pos, NULL);
	at_text = 0;

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


	conf_refresh(ct);
	getch();

}

void
conf_refresh(conft_t *ct)
{
	artl_t	*al;
	int		i;

	/* update status window */
	werase(statwin);
	wprintw(statwin, " CKom: %30s Msgs:%d New:%d Old:%d",
		ct->ct_name, ct->ct_last_text - ct->ct_first_text,
		ct->ct_last_text - ct->ct_first_text, 0);
	wrefresh(statwin);

	/* update thread window */
	conf_thrd_refresh(ct);
	
	/* update command window */
	werase(cmdwin);
	wrefresh(cmdwin);

	/* update text window */
	al = ct->ct_al;
	for (i = 0, at_al_pos = NULL; i <= at_text; i++) {
		assert(cl_art_walk(al, &at_al_pos, &at_art) == 0);
	}
	conf_text_refresh(ct);
}

void
conf_text_refresh(conft_t *ct)
{

	wmove(infowin, 0,0);
	werase(infowin);
	wprintw(infowin, "  %d/%d: %-30.30s | %-30.30s",
		at_text, ct->ct_last_text - ct->ct_first_text,
		at_art->art_from, at_art->art_real_subj);
	wrefresh(infowin);

	werase(textwin);
	wprintw(textwin, "hej hopp ");
	wrefresh(textwin);
}

void
conf_thrd_refresh(conft_t *ct)
{
	artl_t	*al_pos, *al;
	art_t	*art;
	int		i;

	werase(thrdwin);
	al = ct->ct_al;
	al_pos = at_al_pos;
	for (i = 0; i < thrdwin_height; i++) {
		if (cl_art_walk(al, &al_pos, &art) < 0)
			break;
		wmove(thrdwin, i, 0);
		if (i + top_text == at_text)
			wprintw(thrdwin, "->");
		else
			wprintw(thrdwin, "  ");
		wprintw(thrdwin, "%4d ", i + top_text);
		wprintw(thrdwin, "%-20.20s (%4d) %-30.30s",
			art->art_from, art->art_no_of_lines, art->art_subj);
	}
	wrefresh(thrdwin);
}
