
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "rkomsupport.h"

#include "rkom.h"
#include "next.h"
#include "set.h"
#include "backend.h"

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
	if (hln - mr > 0) {
		prompt = PROMPT_NEXT_TEXT;
		return;
	}
	conf = rk_unreadconf(myuid);
	if (conf->ru_confs.ru_confs_len)
		prompt = PROMPT_NEXT_CONF;
	else
		prompt = PROMPT_SEE_TIME;
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
			if (r)
				continue;
			if (rk_is_read(mi[i].rmi_numeric))
				continue;
			break;
		}

	if (i == len) { /* No, nothing followed */
again:
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
}

void
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
}

void
next_text(char *str)
{
	int local, global, rv;

igen:	local = rk_next_unread(curconf, myuid);
	if (local == 0) {
		rprintf("Du har inga mer olästa inlägg.\n");
		next_prompt();
		return;
	}
	global = rk_local_to_global(curconf, local);
	rv = show_text(global, 1);
	mark_read(global);
	next_action(global);
	lastlasttext = lasttext;
	lasttext = global;
	if (rv)
		goto igen;
}

void
next_comment(char *str)
{
	struct rk_text_stat *ts;
	struct rk_misc_info *mi;
	int global, len, i, rv = 0;

igen:	if (pole == 0) {
		if (rv == 0)
back:			rprintf("Det finns ingen nästa kommentar.\n");
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
		goto back;
	}
	if (rk_is_read(mi[i].rmi_numeric))
		goto try;
	global = mi[i].rmi_numeric;
	rv = show_text(global, 1);
	mark_read(global);
	next_action(global);
	lastlasttext = lasttext;
	lasttext = global;
	if (rv)
		goto igen;
}

void
next_marked(char *str)
{
	static int lastseen;
	struct rk_mark_retval *rmr;
	struct rk_marks *rm;
	int i;

	rmr = rk_getmarks();
	rm = rmr->rmr_marks.rmr_marks_val;
	if (rmr->rmr_retval)
		return rprintf("Det sket sej: %s\n", error(rmr->rmr_retval));
	if (rmr->rmr_marks.rmr_marks_len == 0)
		return rprintf("Du har inga markerade inlägg.\n");
	rprintf("(Återse) nästa markerade inlägg.\n");
	if (lastseen == 0) {
		lastseen = rm[0].rm_text;
		show_text(lastseen, 1);
		prompt = PROMPT_NEXT_MARKED;
		return;
	}
	for (i = 0; i < rmr->rmr_marks.rmr_marks_len; i++) {
		if (rm[i].rm_text != lastseen)
			continue;
		if (i+1 == rmr->rmr_marks.rmr_marks_len) {
			lastseen = 0;
			rprintf("Du har slut markerade inlägg.\n");
			next_prompt();
			return;
		}
		prompt = PROMPT_NEXT_MARKED;
		lastseen = rm[i+1].rm_text;
		show_text(lastseen, 1);
		return;
	}
	lastseen = 0;
	next_prompt();
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
		return;
	}
	show_text(mi[i].rmi_numeric, 1);
	lastlasttext = lasttext;
	lasttext = mi[i].rmi_numeric;
}

void
next_resee_root(int text)
{
	struct rk_text_stat *ts;
	struct rk_misc_info *mi;
	int i, len;
	int count;

restart:
	ts = rk_textstat(text);
	mi = ts->rt_misc_info.rt_misc_info_val;
	len = ts->rt_misc_info.rt_misc_info_len;
	count = 0;
	for (i = 0; i < len; i++)
		if (mi[i].rmi_type == comm_to ||
		    mi[i].rmi_type == footn_to) {
			switch(count) {
			case 0:
				text = mi[i].rmi_numeric;
				goto restart;
			case 1:
				next_resee_root(text);
			default:
				next_resee_root(mi[i].rmi_numeric);
				count++;
			}
		}

	if(!count) {
		show_text(text, 1);
		lastlasttext = lasttext;
		lasttext = text;
	}
}

void
next_resee_text_unmodified(int num)
{
	show_text(num, 0);
	lastlasttext = lasttext;
	lasttext = num;
}

void
next_resee_text(int num)
{
	show_text(num, 1);
	lastlasttext = lasttext;
	lasttext = num;
}

void
next_again(char *str)
{
	show_text(lasttext, 1);
}

void
next_hoppa(char *str)
{
	struct rk_text_stat *ts;
	struct rk_misc_info *mi;
	int global, len, i, hoppade;

	hoppade = global = 0;
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
			next_action(global);
			continue;
		}
		global = mi[i].rmi_numeric;
		mark_read(global);
		hoppade++;
		next_action(global);
		lasttext = global;
	}
}

void
next_resee_presentation(char *name)
{
	struct rk_confinfo_retval *rv;
	struct rk_conference *rc;

	rv = match_complain(name, MATCHCONF_PERSON|MATCHCONF_CONF);
	if (rv == 0)
		return;
	printf("Återse presentation (för) %s.\n",
	    rv->rcr_ci.rcr_ci_val[0].rc_name);
	rc = rk_confinfo(rv->rcr_ci.rcr_ci_val[0].rc_conf_no);
	if (rc->rc_retval) {
		printf("Kunde inte läsa presentationen: %s\n",
		    error(rc->rc_retval));
		return;
	}
	if (rc->rc_presentation == 0)
		printf("Det finns ingen presentation.\n");
	else
		show_text(rc->rc_presentation, 1);
	lastlasttext = lasttext;
	lasttext = rc->rc_presentation;
}

void
next_resee_faq(char *name)
{
	struct rk_confinfo_retval *rv;
	struct rk_conference *rc;
	struct rk_aux_item *rai;
	int naux, i;

	rv = match_complain(name, MATCHCONF_CONF);
	if (rv == 0)
		return;
	printf("Återse FAQ (för) %s.\n",
	    rv->rcr_ci.rcr_ci_val[0].rc_name);
	rc = rk_confinfo(rv->rcr_ci.rcr_ci_val[0].rc_conf_no);
	if (rc->rc_retval) {
		printf("Kunde inte läsa FAQn: %s\n",
		    error(rc->rc_retval));
		return;
	}
	naux = rc->rc_aux_item.rc_aux_item_len;
	rai = rc->rc_aux_item.rc_aux_item_val;
	for (i = 0; i < naux; i++) {
		if (rai[i].rai_tag == RAI_TAG_FAQ_TEXT)
			break;
	}
	if (naux == 0 || i == naux)
		rprintf("Det finns ingen FAQ för %s.\n", 
		    rv->rcr_ci.rcr_ci_val[0].rc_name);
	else
		show_text(atoi(rai[i].rai_data), 1);
	lastlasttext = lasttext;
	lasttext = rc->rc_presentation;
}
