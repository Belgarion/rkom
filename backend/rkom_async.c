
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>

#include "rkom_proto.h"
#include "exported.h"
#include "backend.h"

static void async_new_text(void);
/*
 * Handle a async message. Put it on a queue; then leave it to
 * async_handler() to do the rest.
 */
void
async(int level)
{
	int narg, type/*, pers, rcpt*/;
//	char *s, *t;

	narg = get_int();
	type = get_int();

	switch (type) {
	case 15: /* New text created */
		async_new_text();
#if 0
		if (prompt == PROMPT_SEE_TIME)
			prompt = PROMPT_NEXT_CONF;
		else
			return 0;
#endif
		break;

#if 0
	case 9:
	case 13:
		pers = get_int();
		get_eat('\n');
		if (level == 0)
			s = conf_num2name(pers);
		else
			s = conf_num2name_nowait(pers);
		printf("\n%s har just loggat %s.\n", s,
		    (type == 13 ? "ut" : "in"));
		break;

	case 5:
		pers = get_int();
		s = get_string();
		t = get_string();
		get_eat('\n');
		printf("\n%s bytte just namn till %s.\n", s, t);
		conf_delete(s);
		free(s);
		free(t);
		break;

	case 12: /* Unicast/Broadcast/Multicast message */
		rcpt = get_int();
		pers = get_int();
		s = get_string();
		get_eat('\n');
printf("\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		t = (level ? conf_num2name_nowait(pers) : conf_num2name(pers));
		if (rcpt == 0)
			printf("Allmänt meddelande från %s\n\n", t);
		else if (rcpt == myuid)
			printf("Personligt meddelande från %s\n\n", t);
		else
			printf("Meddelande till %d från %s\n\n", rcpt, t);
		printf("%s\n", s);
printf("----------------------------------------------------------------\n");
		break;
#endif

	default:
		get_eat('\n');
		return;
	}
	return;
}

void
async_handle()
{
}

void
async_new_text()
{
	int i, type, cnt, conf, local;
	struct rk_time time;

	get_int();
	read_in_time(&time);
	get_int();get_int();get_int();get_int();
	cnt = get_int();
	get_accept('{');
	for (i = 0; i < cnt; i++) {
		type = get_int();

		switch (type) {
		case rec_time:
		case sentat:
			read_in_time(&time);
			break;

		case recpt:
			conf = get_int();
			if (get_int() != loc_no)
				printf("async_new_text: bad protocol\n");
			local = get_int();
			i++;
			conf_set_high_local(conf, local);
			break;

		default:
			get_int();
			break;
		}
	}
	get_accept('}');
	get_eat('\n');
}
