
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "rkom_proto.h"

#include "rkom.h"
#include "next.h"
#include "set.h"

struct keeptrack {
	struct keeptrack *back;
	int textnr;
	int listidx;
};

int lasttext, lastlasttext;
static struct keeptrack *pole;

/*
 * Set prompt to one suitable for now.
 */
void
next_prompt()
{
	struct rk_unreadconfval *conf;
	struct rk_conference *rkc;
	struct rk_membership *m;
	int hln, mr;

	rkc = rk_confinfo(curconf);
	hln = rkc->rc_first_local_no + rkc->rc_no_of_texts - 1;
	m = rk_membership(myuid, curconf);
	mr = m->rm_last_text_read;
	free(rkc);
	free(m);
	if (hln - mr > 0) {
		prompt = PROMPT_NEXT_TEXT;
		return;
	}
	conf = rk_unreadconf(myuid);
	if (conf->ru_confs.ru_confs_len)
		prompt = PROMPT_NEXT_CONF;
	else
		prompt = PROMPT_SEE_TIME;
	free(conf);
}
/*
 * Next action: decide what to do next is. Choose between:
 *	- Nästa kommentar
 *	- Nästa fotnot
 *	- Nästa inlägg
 *	- Nästa möte
 */
static void
next_action(int nr)
{
	struct keeptrack *kt;
	struct rk_text_stat *ts;
	struct rk_misc_info *mi;
	int i, len;

	ts = rk_textstat(nr);

	/* First: see if there is anything following this text */
	len = ts->rt_misc_info.rt_misc_info_len;
	mi = ts->rt_misc_info.rt_misc_info_val;
	for (i = 0; i < len; i++)
		if (mi[i].rmi_type == footn_in ||
		    mi[i].rmi_type == comm_in) {
			struct rk_text_stat *ts2;
			int r;

			ts2 = rk_textstat(mi[i].rmi_numeric);
			r = ts2->rt_retval;
			free(ts2);
			if (r)
				continue;
			if (rk_is_read(mi[i].rmi_numeric))
				continue;
			break;
		}

	if (i == len) { /* No, nothing followed */
again:		free(ts); /* Forget last text */
		if (pole == 0) { /* Nothing to do at all */
			next_prompt();
			return;
		}
		ts = rk_textstat(pole->textnr);
		len = ts->rt_misc_info.rt_misc_info_len;
		mi = ts->rt_misc_info.rt_misc_info_val;

		for (i = pole->listidx + 1; i < len; i++)
			if (mi[i].rmi_type == footn_in ||
			    mi[i].rmi_type == comm_in) {
				struct rk_text_stat *ts2;
				int r;

				ts2 = rk_textstat(mi[i].rmi_numeric);
				r = ts2->rt_retval;
				free(ts2);
				if (r)
					continue;
				if (rk_is_read(mi[i].rmi_numeric))
					continue;
				break;  
			}
		if (i == len) { /* last text at this leaf, iterate */
			struct keeptrack *old;

			old = pole;
			pole = pole->back;
			free(old);
			goto again;
		}
		pole->listidx = i;
	} else {
		/* More to read, just follow */
		kt = calloc(sizeof(struct keeptrack), 1);
		kt->textnr = nr;
		kt->listidx = i;
		kt->back = pole;
		pole = kt;
	}
	if (iseql("read-depth-first", "1"))
		prompt = PROMPT_NEXT_COMMENT;
	else
		prompt = PROMPT_NEXT_TEXT;
	free(ts);
}

/*
 * Deletes the comment chain.
 */
void
next_resetchain()
{
	struct keeptrack *kt;

	while (pole) {
		kt = pole->back;
		free(pole);
		pole = kt;
	}
	next_prompt();
}

void
next_conf(char *str)
{
	struct rk_unreadconfval *retval;
	struct rk_conference *conf;
	struct rk_membership *member;
	int *unread, i, len;

	retval = rk_unreadconf(myuid);
	unread = retval->ru_confs.ru_confs_val;
	len = retval->ru_confs.ru_confs_len;
	if (len == 0) {
		rprintf("Du har inga olästa inlägg.\n");
		prompt = PROMPT_SEE_TIME;
		return;
	}
	for (i = 0; i < len; i++)
		if (unread[i] == curconf)
			break;
	if (i+1 < len)
		curconf = unread[i+1];
	else
		curconf = unread[0];

	conf = rk_confinfo(curconf);
	member = rk_membership(myuid, curconf);
	rk_change_conference(curconf);

	rprintf("Nästa möte: %s\n\n", conf->rc_name);
	rprintf("Du har %d olästa inlägg.\n", conf->rc_first_local_no +
	    conf->rc_no_of_texts - member->rm_last_text_read - 1 -
	    member->rm_read_texts.rm_read_texts_len);
	prompt = PROMPT_NEXT_TEXT;
	free(conf);
	free(member);
}

static void
mark_read(int nr)
{
	struct rk_text_stat *ts;
	struct rk_misc_info *mi;
	int i, len;

	ts = rk_textstat(nr);
	len = ts->rt_misc_info.rt_misc_info_len;
	mi = ts->rt_misc_info.rt_misc_info_val;
	for (i = 0; i < len; i++) {
		if (mi[i].rmi_type != recpt && mi[i].rmi_type != cc_recpt)
			continue;
		if (mi[i+1].rmi_type != loc_no)
			continue;
		if (ismember(mi[i].rmi_numeric))
			rk_mark_read(mi[i].rmi_numeric, mi[i+1].rmi_numeric);
	}
	free(ts);
}

void
next_text(char *str)
{
	int local, global;

	local = rk_next_unread(curconf, myuid);
	if (local == 0) {
		rprintf("Du har inga mer olästa inlägg.\n");
		next_prompt();
		return;
	}
	global = rk_local_to_global(curconf, local);
	show_text(global);
	mark_read(global);
	next_action(global);
	lastlasttext = lasttext;
	lasttext = global;
}

void
next_comment(char *str)
{
	struct rk_text_stat *ts;
	struct rk_misc_info *mi;
	int global, len, i;

	if (pole == 0) {
back:		rprintf("Det finns ingen nästa kommentar.\n");
		prompt = PROMPT_NEXT_TEXT;
		return;
	}
	ts = rk_textstat(pole->textnr);
	len = ts->rt_misc_info.rt_misc_info_len;
	mi = ts->rt_misc_info.rt_misc_info_val;
try:	for (i = pole->listidx; i < len; i++)
		if (mi[i].rmi_type == footn_in ||
		    mi[i].rmi_type == comm_in)
			break;
	if (i == len) {
		free(ts);
		goto back;
	}
	if (rk_is_read(mi[i].rmi_numeric))
		goto try;
	global = mi[i].rmi_numeric;
	free(ts);
	show_text(global);
	mark_read(global);
	next_action(global);
	lastlasttext = lasttext;
	lasttext = global;
}

void
next_resee_comment()
{
	struct rk_text_stat *ts;
	struct rk_misc_info *mi;
	int i, len;

	ts = rk_textstat(lasttext);
	mi = ts->rt_misc_info.rt_misc_info_val;
	len = ts->rt_misc_info.rt_misc_info_len;
	for (i = 0; i < len; i++)
		if (mi[i].rmi_type == comm_to ||
		    mi[i].rmi_type == footn_to)
			break;
	if (i == len) {
		rprintf("Inlägget är varken kommentar eller fotnot.\n");
		free(ts);
		return;
	}
	show_text(mi[i].rmi_numeric);
	lastlasttext = lasttext;
	lasttext = mi[i].rmi_numeric;
	free(ts);
}

void
next_resee_text(int num)
{
	show_text(num);
	lastlasttext = lasttext;
	lasttext = num;
}

void
next_again(char *str)
{
	show_text(lasttext);
}

void
next_hoppa(char *str)
{
	struct rk_text_stat *ts;
	struct rk_misc_info *mi;
	int global, len, i, hoppade;

	hoppade = 0;
	while (1) {
		if (pole == 0) {
			if (hoppade == 0)
				rprintf("Du hoppade inte över några inlägg.\n");
			else
				rprintf("Du hoppade över %d inlägg.\n", hoppade);
			return;
		}
		ts = rk_textstat(pole->textnr);
		len = ts->rt_misc_info.rt_misc_info_len;
		mi = ts->rt_misc_info.rt_misc_info_val;
		for (i = pole->listidx; i < len; i++)
			if (mi[i].rmi_type == footn_in ||
			    mi[i].rmi_type == comm_in)
				break;
		if (i == len) {
			free(ts);
			next_action(global);
			continue;
		}
		global = mi[i].rmi_numeric;
		free(ts);
		mark_read(global);
		hoppade++;
		next_action(global);
		lasttext = global;
	}
}
