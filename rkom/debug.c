
#include <sys/types.h>
#include <stdio.h>

#include "backend.h"
#include "rkom.h"
#include "debug.h"

void
debug_dump_membership(char *name)
{
	struct rk_confinfo *rv;
	struct rk_membership *rm;
	int i;

	if ((rv = match_complain(name, MATCHCONF_CONF)) == NULL)
		return;

	rm = rk_membership(myuid, rv[0].rc_conf_no);
	rprintf("Debuginfo membership för %s.\n", rv[0].rc_name);
	rprintf("Position\t%d\n", rm->rm_position);
	rprintf("Last read\t%s\n", get_date_string(&rm->rm_last_time_read));
	rprintf("Conference\t%d\n", rm->rm_conference);
	rprintf("Priority\t%d\n", rm->rm_priority);
	rprintf("Last read text\t%d\n", rm->rm_last_text_read);
	rprintf("Read texts: ");
	for (i = 0; i < rm->rm_read_texts_len; i++)
		rprintf("%d ", rm->rm_read_texts_val[i]);
	rprintf("\n");
	rprintf("Added by\t%d\n", rm->rm_added_by);
	rprintf("Added at\t%s\n", get_date_string(&rm->rm_added_at));
	rprintf("Type\t\t%d\n", rm->rm_type);
}

void
debug_dump_conference(char *name)
{
	struct rk_confinfo *rv;
	struct rk_conference *rc;
	struct rk_uconference *uc;

	if ((rv = match_complain(name, MATCHCONF_CONF)) == NULL)
		return;

	rc = rk_confinfo(rv[0].rc_conf_no);
	rprintf("Debuginfo conference för %s.\n", rc->rc_name);
	rprintf("Type\t%d\n", rc->rc_type);
	rprintf("Created at:\t%s\n", get_date_string(&rc->rc_creation_time));
	rprintf("Last written at:\t%s\n", get_date_string(&rc->rc_last_written));
	rprintf("Creator\t%d\n", rc->rc_creator);
	rprintf("Presentation\t%d\n", rc->rc_presentation);
	rprintf("Supervisor\t%d\n", rc->rc_supervisor);
	rprintf("Perm. Sub.\t%d\n", rc->rc_permitted_submitters);
	rprintf("Superconf\t%d\n", rc->rc_super_conf);
	rprintf("Msgofday\t%d\n", rc->rc_msg_of_day);
	rprintf("Nice\t\t%d\n", rc->rc_nice);
	rprintf("Keepcommented\t%d\n", rc->rc_keep_commented);
	rprintf("No of members\t%d\n", rc->rc_no_of_members);
	rprintf("First local no\t%d\n", rc->rc_first_local_no);
	rprintf("No of texts\t%d\n", rc->rc_no_of_texts);
	rprintf("Expires\t\t%d\n", rc->rc_expire);

	uc = rk_uconfinfo(rv[0].rc_conf_no);
	rprintf("Debuginfo uconference för %s.\n", uc->ru_name);
	rprintf("Type\t\t%d\n", uc->ru_type);
	rprintf("Highest local no\t%d\n", uc->ru_highest_local_no);
	rprintf("Nice\t\t%d\n", uc->ru_nice);
}
