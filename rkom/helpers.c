
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
