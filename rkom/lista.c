
#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>
#ifdef LINUX
#include <string.h>
#endif
#include <strings.h>

#include "rkomsupport.h"
#include "rkom_proto.h"
#include "rkom.h"
#include "list.h"
#include "next.h"
#include "set.h"

void
list_comm(char *args)
{
	rprintf(
#include "kommandon"
	);
}

void
list_conf(char *str)
{
	struct rk_confinfo_retval *rv;
	struct rk_confinfo *ci;
	int i, nconfs;

	rv = rk_matchconf("", MATCHCONF_CONF);
	nconfs = rv->rcr_ci.rcr_ci_len;
	if (nconfs == 0)
		return;
	ci = rv->rcr_ci.rcr_ci_val;

	rprintf("\nSenaste inlägg   Medl. Tot Inl   Namn (typ)\n");
	for (i = 0; i < nconfs; i++) {
		struct rk_conference *C;
		struct rk_membership *M = NULL; /* GCC braino */

		C = rk_confinfo(ci[i].rc_conf_no);
		if (myuid)
			M = rk_membership(myuid, ci[i].rc_conf_no);

		rprintf("%s %4d %7d %s  %s\n",
		    get_date_string(&C->rc_last_written),
		    C->rc_no_of_members, C->rc_first_local_no +
		    C->rc_no_of_texts - 1, (myuid == 0 ? " " :
		    (C->rc_supervisor == myuid ? "O" :
		    (M->rm_retval ? " " : "*" ))), ci[i].rc_name);

		free(C);
		if (myuid)
			free(M);
		if (discard)
			break;
	}
	rprintf("\n");
	free(rv);
}

void
list_news(char *args)
{
	struct rk_unreadconfval *conf;
	int i, nconf, *confs;
	int longfmt;

	if (myuid == 0) {
		rprintf("Du måste logga in först.\n");
		return;
	}
	/*
	 * Get number of unread texts.
	 */
	conf = rk_unreadconf(myuid);

	longfmt = iseql("unread-long-format","1");
	
	/* Show where we have unread texts. */
	nconf = conf->ru_confs.ru_confs_len;
	if (nconf) {
		confs = conf->ru_confs.ru_confs_val;

		for (i = 0; i < nconf; i++) {
			struct rk_conference *rkc;
			struct rk_membership *m;
			int hln, nr;

			rkc = rk_confinfo(confs[i]);

			if (rkc->rc_retval) {
				rprintf("%d sket sej med %d\n",
				    confs[i], rkc->rc_retval);
				free(rkc);
				continue;
			}
			hln = rkc->rc_first_local_no + rkc->rc_no_of_texts - 1;
			m = rk_membership(myuid, confs[i]);

			if (m->rm_retval) {
				rprintf("%d,%d sket sej med %d\n",
				    confs[i], myuid, m->rm_retval);
				free(rkc);
				free(m);
				continue;
			}
			nr = hln - m->rm_last_text_read;
			if(longfmt)
				rprintf("Du har %d oläst%s inlägg av %d i %s\n",
				    nr, nr == 1 ? "" : "a", hln, rkc->rc_name);
			else
				rprintf("Du har %d oläst%s inlägg i %s\n",
				    nr, nr == 1 ? "" : "a", rkc->rc_name);
			free(rkc);
			free(m);
		}
		rprintf("\n");
	} else
		rprintf("Du har inga olästa inlägg.\n");
	free(conf);
}

static int
sanitycheck(char *str)
{
	int text;

	if (myuid == 0) {
		rprintf("Du måste logga in först.\n");
		return 0;
	}
	if (str == 0) {
		text = lasttext;
		if (text == 0)
			rprintf("Det har ännu inte läst någon text.\n");
	} else if ((text = atoi(str)) == 0) {
		rprintf("%s är ett dåligt inläggsnummer.\n", str);
		return 0;
	}
	return text;
}

void
list_marked(char *str)
{
	struct rk_mark_retval *rmr;
	struct rk_marks *rm;
	int i;

	if (myuid == 0) {
		rprintf("Du måste logga in först.\n");
		return;
	}
	rmr = rk_getmarks();
	rm = rmr->rmr_marks.rmr_marks_val;
	if (rmr->rmr_retval) {
		rprintf("Det sket sej: %s\n", error(rmr->rmr_retval));
	} else if (rmr->rmr_marks.rmr_marks_len == 0) {
		rprintf("Du har inga markerade inlägg.\n");
	} else {
		rprintf("Inläggsnummer\tPrioritet\n");
		for (i = 0; i < rmr->rmr_marks.rmr_marks_len; i++)
			rprintf("%d\t\t%d\n", rm[i].rm_text, rm[i].rm_type);
		rprintf("\n");
	}
	free(rmr);
}

void    
list_mark(char *str)
{
	int text, stat;

	if ((text = sanitycheck(str)) == 0)
		return;

	stat = rk_setmark(text, 100);
	if (stat)
		rprintf("Det sket sej: %s\n", error(stat));
	else
		rprintf("Du har nu markerat inlägg %d.\n", text);
}

void
list_unmark(char *str)
{
	int text, stat;

	if ((text = sanitycheck(str)) == 0)
		return;
	stat = rk_unmark(text);
	if (stat)
		rprintf("Det sket sej: %d\n", stat);
	else
		rprintf("Du har nu avmarkerat inlägg %d.\n", text);
}

void
list_subject()
{
	struct rk_conference *conf;
	struct rk_text_stat *ts;
	int high, low, i, nr, rows;
	char *gubbe, *text, *c;

	conf = rk_confinfo(curconf);
	if (conf->rc_retval) {
		rprintf("rk_confinfo sket sej: %s\n", error(conf->rc_retval));
		return;
	}
	rprintf("Lista ärenden\n");
	rprintf("Inlägg\tDatum\t  Författare           Ärende\n");
	high = conf->rc_no_of_texts + conf->rc_first_local_no - 1;
	low = conf->rc_first_local_no;
	free(conf);
	rows = 4;
	for (i = high; i >= low; i--) {
		nr = rk_local_to_global(curconf, i);
		if (nr == 0)
			continue;
		ts = rk_textstat(nr);
		text = rk_gettext(nr);
		gubbe = vem(ts->rt_author);
		rprintf("%d\t%d", nr, ts->rt_time.rt_year + 1900);
		rprintf("%s%d", ts->rt_time.rt_month > 8 ? "" : "0",
		    ts->rt_time.rt_month + 1);
		rprintf("%s%d  ", ts->rt_time.rt_day > 9 ? "" : "0",
		    ts->rt_time.rt_day);
		if (strlen(gubbe) > 20)
			gubbe[20] = 0;
		if ((c = index(text, '\n')))
			*c = 0;
		if (strlen(text) > 40)
			text[40] = 0;
		rprintf("%-21s%s\n", gubbe, text);
		free(ts);
		free(text);
	}
}

void
list_unread()
{
	struct rk_membership *rm;
	struct rk_conference *conf;
	struct rk_text_stat *ts;
	int high, low, i, nr, rows;
	char *gubbe, *text, *c;

	conf = rk_confinfo(curconf);
	if (conf->rc_retval) {
		rprintf("rk_confinfo sket sej: %s\n", error(conf->rc_retval));
		free(conf);
		return;
	}
	rprintf("Lista olästa (ärenden)\n");
	rprintf("Inlägg\tDatum\t  Författare           Ärende\n");
	rm = rk_membership(myuid, curconf);
	if (rm->rm_retval) {
		rprintf("rk_membership sket sej: %s\n", error(rm->rm_retval));
		free(conf);
		free(rm);
		return;
	}
	high = conf->rc_no_of_texts + conf->rc_first_local_no - 1;
	low = rm->rm_last_text_read + 1;
	free(conf);
	free(rm);
	rows = 4;
	for (i = high; i >= low; i--) {
		nr = rk_local_to_global(curconf, i);
		if (nr == 0)
			continue;
		ts = rk_textstat(nr);
		text = rk_gettext(nr);
		gubbe = vem(ts->rt_author);
		rprintf("%d\t%d", nr, ts->rt_time.rt_year + 1900);
		rprintf("%s%d", ts->rt_time.rt_month > 8 ? "" : "0",
		    ts->rt_time.rt_month + 1);
		rprintf("%s%d  ", ts->rt_time.rt_day > 9 ? "" : "0",
		    ts->rt_time.rt_day);
		if (strlen(gubbe) > 20)
			gubbe[20] = 0;
		if ((c = index(text, '\n')))
			*c = 0;
		if (strlen(text) > 40)
			text[40] = 0;
		rprintf("%-21s%s\n", gubbe, text);
		free(ts);
		free(text);
	}
}
