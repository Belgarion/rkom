
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(AIX)
#include <strings.h>
#endif
#include <string.h>

#include "rkomsupport.h"
#include "rkom.h"
#include "backend.h"

static int
directmatch(struct rk_confinfo_retval *r, char *n)
{
	int num = r->rcr_ci.rcr_ci_len;
	int i, m = 0, nr = 0;

	for (i = 0; i < num; i++)
		if (strcasecmp(r->rcr_ci.rcr_ci_val[i].rc_name, n) == 0) {
			nr++;
			m = i;
		}

	if (nr != 1)
		return 0;
	if (m != 0) {
		r->rcr_ci.rcr_ci_val[0].rc_name =
		    r->rcr_ci.rcr_ci_val[m].rc_name;
		r->rcr_ci.rcr_ci_val[0].rc_type =
		    r->rcr_ci.rcr_ci_val[m].rc_type;
		r->rcr_ci.rcr_ci_val[0].rc_conf_no =
		    r->rcr_ci.rcr_ci_val[m].rc_conf_no;
	}
	r->rcr_ci.rcr_ci_len = 1;
	return 1;
}

struct rk_confinfo_retval *
match_complain(char *str, int type)
{
	struct rk_confinfo_retval *retval;
	int i, num;

	retval = rk_matchconf(str, type);
	num = retval->rcr_ci.rcr_ci_len;

	if (num == 0) {
		rprintf("Det finns ingenting som matchar \"%s\".\n", str);
		return 0;
	} else if (num > 1) {
		if (directmatch(retval, str))
			return retval;
		rprintf("Texten \"%s\" är flertydig. Du kan mena:\n", str);
		for (i = 0; i < num; i++)
			rprintf("%s\n", retval->rcr_ci.rcr_ci_val[i].rc_name);
		rprintf("\n");
		return 0;
	}
	return retval;
}

int
ismember(int conf)
{
	struct rk_memberconflist *mcl;
	int i, rv = 0;

	mcl = rk_memberconf(myuid);
	if (mcl->rm_retval == 0)
		for (i = 0; i < mcl->rm_confs.rm_confs_len; i++)
			if (mcl->rm_confs.rm_confs_val[i] == conf) {
				rv = 1;
				break;
			}
	return rv;
}
