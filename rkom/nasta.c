
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "rkom_proto.h"

// #include "conf.h"
// #include "komtyp.h"
#include "rkom.h"
#include "next.h"

int lasttext;

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
		printf("Du har inga olästa inlägg.\n");
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

	printf("Nästa möte: %s\n\n", conf->rc_name);
	printf("Du har %d olästa inlägg.\n", conf->rc_first_local_no +
	    conf->rc_no_of_texts - member->rm_last_text_read - 1);
	prompt = PROMPT_NEXT_TEXT;
	free(conf);
	free(member);
}

void
next_text(char *str)
{
	int local, global;

	local = rk_next_unread(curconf, myuid);
	if (local == 0) {
		printf("Du har inga mer olästa inlägg.\n");
		prompt = PROMPT_NEXT_CONF;
		return;
	}
	global = rk_local_to_global(curconf, local);
	show_text(global);
	lasttext = global;
	rk_mark_read(curconf, local);
}
