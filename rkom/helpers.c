
#include <stdio.h>
#include <stdlib.h>

#include "rkom_proto.h"
#include "rkom.h"

struct rk_confinfo_retval *
match_complain(char *str, int type)
{
	struct rk_confinfo_retval *retval;
	int i, num;

	retval = rk_matchconf(str, type);
	num = retval->rcr_ci.rcr_ci_len;

	if (num == 0) {
		printf("Det finns ingenting som matchar \"%s\".\n", str);
		free(retval);
		return 0;
	} else if (num > 1) {
		printf("Texten \"%s\" är flertydig. Du kan mena:\n", str);
		for (i = 0; i < num; i++)
			printf("%s\n", retval->rcr_ci.rcr_ci_val[i].rc_name);
		printf("\n");
		free(retval);
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
	free(mcl);
	return rv;
}
