
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
directmatch(struct rk_confinfo *r, char *n)
{
	int i, m = 0, nr = 0;

	for (i = 0; r[i].rc_name; i++)
		if (strcasecmp(r[i].rc_name, n) == 0) {
			nr++;
			m = i;
		}

	if (nr != 1)
		return 0;
	if (m != 0) {
		r[0].rc_name = r[m].rc_name;
		r[0].rc_type = r[m].rc_type;
		r[0].rc_conf_no = r[m].rc_conf_no;
	}
	return 1;
}

struct rk_confinfo *
match_complain(char *str, int type)
{
	struct rk_confinfo *rv;
	int i;

	if ((rv = rk_matchconf(str, type)) == NULL) {
		rprintf("Det finns ingenting som matchar \"%s\".\n", str);
	} else if (rv[1].rc_name != NULL) { /* More than one */
		if (directmatch(rv, str))
			return rv;
		rprintf("Texten \"%s\" är flertydig. Du kan mena:\n", str);
		for (i = 0; rv[i].rc_name; i++)
			rprintf("%s\n", rv[i].rc_name);
		rprintf("\n");
		return NULL;
	}
	return rv;
}

int
ismember(int conf)
{
	u_int32_t *list;
	int i;

	if ((list = rk_memberconf(myuid)) == NULL)
		return 0;

	for (i = 0; list[i]; i++)
		if (list[i] == conf)
			return 1;
	return 0;
}
