
#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include "rkom_proto.h"
#include "exported.h"
#include "rkom.h"
#include "list.h"
#include "next.h"

void
list_comm(char *args)
{
	printf(
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

	printf("\nSenaste inlägg   Medl. Tot Inl   Namn (typ)\n");
	for (i = 0; i < nconfs; i++) {
		struct rk_conference *C;
		struct rk_membership *M;

		C = rk_confinfo(ci[i].rc_conf_no);
		if (myuid)
			M = rk_membership(myuid, ci[i].rc_conf_no);

		printf("%s %4d %7d %s  %s\n",
		    get_date_string(&C->rc_last_written),
		    C->rc_no_of_members, C->rc_first_local_no +
		    C->rc_no_of_texts - 1, (myuid == 0 ? " " :
		    (C->rc_supervisor == myuid ? "O" :
		    (M->rm_retval ? " " : "*" ))), ci[i].rc_name);

		free(C);
		if (myuid)
			free(M);
	}
	printf("\n");
	free(rv);
}

void
list_news(char *args)
{
	struct rk_unreadconfval *conf;
	int i, nconf, *confs;

	if (myuid == 0) {
		printf("Du måste logga in först.\n");
		return;
	}
	/*
	 * Get number of unread texts.
	 */
	conf = rk_unreadconf(myuid);
	
	/* Show where we have unread texts. */
	nconf = conf->ru_confs.ru_confs_len;
	if (nconf) {
		confs = conf->ru_confs.ru_confs_val;

		for (i = 0; i < nconf; i++) {
			struct rk_conference *rkc;
			struct rk_membership *m;
			int hln;

			rkc = rk_confinfo(confs[i]);

			if (rkc->rc_retval) {
				printf("%d sket sej med %d\n",
				    confs[i], rkc->rc_retval);
				free(rkc);
				continue;
			}
			hln = rkc->rc_first_local_no + rkc->rc_no_of_texts - 1;
			m = rk_membership(myuid, confs[i]);

			if (m->rm_retval) {
				printf("%d,%d sket sej med %d\n",
				    confs[i], myuid, m->rm_retval);
				free(rkc);
				free(m);
				continue;
			}
			printf("Du har %d olästa inlägg av %d i %s\n",
			    hln - m->rm_last_text_read, hln, rkc->rc_name);
			free(rkc);
			free(m);
		}
		printf("\n");
	}
}

static int
sanitycheck(char *str)
{
	int text;

	if (myuid == 0) {
		printf("Du måste logga in först.\n");
		return 0;
	}
	if (str == 0) {
		text = lasttext;
		if (text == 0)
			printf("Det har ännu inte läst någon text.\n");
	} else if ((text = atoi(str)) == 0) {
		printf("%s är ett dåligt inläggsnummer.\n", str);
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
		printf("Du måste logga in först.\n");
		return;
	}
	rmr = rk_getmarks(0);
	rm = rmr->rmr_marks.rmr_marks_val;
	if (rmr->rmr_retval) {
		printf("Det sket sej: %s\n", error(rmr->rmr_retval));
	} else if (rmr->rmr_marks.rmr_marks_len == 0) {
		printf("Du har inga markerade inlägg.\n");
	} else {
		printf("Inläggsnummer\tPrioritet\n");
		for (i = 0; i < rmr->rmr_marks.rmr_marks_len; i++)
			printf("%d\t\t%d\n", rm[i].rm_text, rm[i].rm_type);
		printf("\n");
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
		printf("Det sket sej: %s\n", error(stat));
	else
		printf("Du har nu markerat inlägg %d.\n", text);
}

void
list_unmark(char *str)
{
	int text, stat;

	if ((text = sanitycheck(str)) == 0)
		return;
	stat = rk_unmark(text);
	if (stat)
		printf("Det sket sej: %d\n", stat);
	else
		printf("Du har nu avmarkerat inlägg %d.\n", text);
}
