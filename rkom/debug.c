
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
