
#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>
#ifdef LINUX
#include <string.h>
#endif
#include <strings.h>

#include "rkomsupport.h"
#include "rkom.h"
#include "list.h"
#include "next.h"
#include "set.h"
#include "backend.h"

void
list_comm(char *args)
{
	rprintf(
#include "kommandon"
	);
}

void
list_conf_q(char *str)
{
	struct rk_confinfo *ci;
	int i;

	if ((ci = rk_matchconf("", MATCHCONF_CONF)) == NULL)
		return;

	rprintf("\nTyp       Mötesnamn\n");
	for (i = 0; ci[i].rc_name; i++) {
		struct rk_uconference *UC;
		if ((UC = rk_uconfinfo(ci[i].rc_conf_no)) == NULL)
			continue;

		rprintf("%08d  %s\n", UC->ru_type, UC->ru_name);
	}
	rprintf("\n");
}

void
list_conf(char *str)
{
	struct rk_confinfo *ci;
	int i;

	if ((ci = rk_matchconf("", MATCHCONF_CONF)) == NULL)
		return;

	rprintf("\nSenaste inlägg   Medl. Tot Inl   Namn (typ)\n");
	for (i = 0; ci[i].rc_name; i++) {
		struct rk_conference *C;
		struct rk_membership *M = NULL; /* GCC braino */

		if ((C = rk_confinfo(ci[i].rc_conf_no)) == NULL) {
			rprintf("Kunde inte läsa confinfon för möte %d: %s\n",
			    ci[i].rc_conf_no, error(komerr));
			continue;
		}
		if (myuid)
			M = rk_membership(myuid, ci[i].rc_conf_no);

		rprintf("%s %4d %7d %s  %s\n",
		    get_date_string(&C->rc_last_written),
		    C->rc_no_of_members, C->rc_first_local_no +
		    C->rc_no_of_texts - 1, (myuid == 0 ? " " :
		    (C->rc_supervisor == myuid ? "O" :
		    (M ? " " : "*" ))), ci[i].rc_name);

		if (discard)
			break;
	}
	rprintf("\n");
}

void
list_priorities(char *foo)
{
	u_int32_t *list;
	int i;

	if ((list = rk_memberconf(myuid)) == NULL) {
		rprintf("rk_memberconf: %s\n", error(komerr));
		return;
	}

	rprintf("\nLista prioriteter (för möten jag är medlem i)\n");
	rprintf("Prioritet\tMötesnamn\n");
	for (i = 0; list[i]; i++) {
		struct rk_membership *rkm;
		struct rk_conference *rc;

		rkm = rk_membership(myuid, list[i]);
		rc = rk_confinfo(list[i]);
		rprintf("%8d\t%s\n", rkm->rm_priority, rc->rc_name);
	}
	rprintf("\n");
}

void
list_conf_members(char *str)
{
	struct rk_confinfo *ci;
	struct rk_member *rm;
	struct rk_uconference *ru;
	int i;

	if ((ci = match_complain(str, MATCHCONF_CONF)) == NULL)
		return;
	if ((rm = rk_get_membership(ci->rc_conf_no)) == NULL) {
		rprintf("rk_get_membership sket sej: %s\n", error(komerr));
		return;
	}
	ru = rk_uconfinfo(ci->rc_conf_no);
	rprintf("%s har följande medlemmar:\n", ci->rc_name);
	rprintf("Senast inne\t  Osett\t  Namn\n");
	for (i = 0; rm[i].rm_member; i++) {
		struct rk_conference *rp = rk_confinfo(rm[i].rm_member);
		struct rk_membership *rkm =
		    rk_membership(rm[i].rm_member, ci->rc_conf_no);

		rprintf("%s%7d\t  %s\n",
		    get_date_string(&rkm->rm_last_time_read),
		    ru->ru_highest_local_no - rkm->rm_last_text_read,
		    rp ? rp->rc_name : "(Okänd)");
	}
}

void
list_news(char *args)
{
	int i;
	int longfmt;
	int unr_texts, unr_confs;
	u_int32_t *c;

	if (myuid == 0) {
		rprintf("Du måste logga in först.\n");
		return;
	}
	/*
	 * Get number of unread texts.
	 */
	c = rk_unreadconf(myuid);

	longfmt = iseql("unread-long-format","1");
	
	unr_texts = unr_confs = 0;

	/* Show where we have unread texts. */
	if (c[0]) {
		for (i = 0; c[i]; i++) {
			struct rk_conference *rkc;
			struct rk_membership *m;
			int hln, nr;

			if ((rkc = rk_confinfo(c[i])) == NULL) {
				rprintf("%d sket sej: %s\n",
				    c[i], error(komerr));
				continue;
			}
			hln = rkc->rc_first_local_no + rkc->rc_no_of_texts - 1;
			if ((m = rk_membership(myuid, c[i])) == NULL) {
				rprintf("%d,%d sket sej: %s\n",
				    c[i], myuid, error(komerr));
				continue;
			}
			nr = hln - m->rm_last_text_read;
			if(longfmt)
				rprintf("Du har %d oläst%s inlägg av %d i %s\n",
				    nr, nr == 1 ? "" : "a", hln, rkc->rc_name);
			else
				rprintf("Du har %d oläst%s inlägg i %s\n",
				    nr, nr == 1 ? "" : "a", rkc->rc_name);
			unr_texts += nr;
			unr_confs++;
		}
		rprintf("\nDu har %d oläst%s inlägg i %d möte%s.\n",
		        unr_texts, unr_texts>1?"a":"",
		        unr_confs, unr_confs>1?"n":"");
	} else
		rprintf("Du har inga olästa inlägg.\n");
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
	struct rk_marks *rm;
	int i, len;
	char *c, *c2, buf[50];

	if (myuid == 0) {
		rprintf("Du måste logga in först.\n");
		return;
	}
	if ((rm = rk_getmarks()) == NULL)
		return rprintf("Det sket sej: %s\n", error(komerr));
	if (rm[0].rm_text == 0) {
		rprintf("Du har inga markerade inlägg.\n");
	} else {
		rprintf("Inläggsnummer\tPrioritet\tÄrenderad\n");
		for (i = 0; rm[i].rm_text; i++) {
			rprintf("%d\t\t%d\t\t", rm[i].rm_text, rm[i].rm_type);
			if ((c = rk_gettext(rm[i].rm_text)) == NULL) {
				rprintf("(Ärenderaden fick ej läsas)\n");
				continue;
			}
			if ((c2 = strchr(c, '\n')) == NULL)
				len = strlen(c);
			else
				len = c2 - c;
			if (len > 45)
				len = 45;
			memcpy(buf, c, len);
			buf[len] = 0;
			rprintf("%s\n", buf);
		}
		rprintf("\n");
	}
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

	if ((conf = rk_confinfo(curconf)) == NULL)
		return rprintf("rk_confinfo sket sej: %s\n", error(komerr));
	rprintf("Lista ärenden\n");
	rprintf("Inlägg\tDatum\t  Författare           Ärende\n");
	high = conf->rc_no_of_texts + conf->rc_first_local_no - 1;
	low = conf->rc_first_local_no;
	rows = 4;
	for (i = high; i >= low; i--) {
		nr = rk_local_to_global(curconf, i);
		if (nr == 0)
			continue;
		if ((ts = rk_textstat(nr)) == NULL) {
			rprintf("Stat på text %d misslyckades.\n", nr);
			continue;
		}
		text = strdup(rk_gettext(nr));
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

	if ((conf = rk_confinfo(curconf)) == NULL)
		return rprintf("rk_confinfo sket sej: %s\n", error(komerr));
	rprintf("Lista olästa (ärenden)\n");
	rprintf("Inlägg\tDatum\t  Författare           Ärende\n");
	if ((rm = rk_membership(myuid, curconf)) == NULL) {
		rprintf("rk_membership sket sej: %s\n", error(komerr));
		return;
	}
	high = conf->rc_no_of_texts + conf->rc_first_local_no - 1;
	low = rm->rm_last_text_read + 1;
	rows = 4;
	for (i = high; i >= low; i--) {
		nr = rk_local_to_global(curconf, i);
		if (nr == 0)
			continue;
		if ((ts = rk_textstat(nr)) == NULL) {
			rprintf("Stat på text %d misslyckades.\n", nr);
			continue;
		}
		text = strdup(rk_gettext(nr));
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
	}
}
