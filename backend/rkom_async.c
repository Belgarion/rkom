
#include <sys/types.h>

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "rkom_proto.h"
#include "exported.h"
#include "backend.h"

static void async_new_text(void);

struct mesg {
	struct mesg *next;
	int type;
	int conf;
	int pers;
	char *msg;
};
static struct mesg *pole;

/*
 * Handle a async message. Put it on a queue; then leave it to
 * async_handler() to do the rest.
 */
void
async(int level)
{
	struct mesg *m;
	int narg, type/*, pers, rcpt*/;
//	char *s, *t;

	narg = get_int();
	type = get_int();

	switch (type) {
	case 15: /* New text created */
		async_new_text();
		m = malloc(sizeof(struct mesg));
		m->type = type;
		m->next = pole;
		m->msg = "";
		pole = m;
		break;

	case 12: /* async-send-message */
		m = malloc(sizeof(struct mesg));
		m->type = type; 
		m->conf = get_int();
		m->pers = get_int();
		m->msg = get_string();
		get_eat('\n');
		m->next = pole;
		pole = m;
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
#endif

	default:
		get_eat('\n');
		return;
	}
	if (level == 1)
		async_handle();
}

static int prefetch;

void
async_handle()
{
	struct rk_text_stat *ts;
	extern int fepid;

	if (pole)
		kill(fepid, SIGIO);
	if (prefetch) {
		ts = rk_textstat_server(prefetch);
		free(ts);
		prefetch = 0;
	}
}

struct rk_async *
rk_async_server(u_int32_t arg)
{
	struct mesg *m;
	struct rk_async *ra;
	int size;

	if (pole) {
		size = sizeof(struct rk_async);
		if (pole->type == 12)
			size += strlen(pole->msg) + 1;
		ra = calloc(size, 1);
		ra->ra_type = pole->type;
		ra->ra_conf = pole->conf;
		ra->ra_sender = pole->pers;
		ra->ra_message = (void *)&ra[1];
		strcpy(ra->ra_message, pole->msg);
		if (pole->type == 12)
			free(pole->msg);
		m = pole;
		pole = pole->next;
		free(m);
	} else {
		ra = calloc(sizeof(struct rk_async), 1);
		ra->ra_message = "";
	}
	return ra;
}

void
async_new_text()
{
	int i, type, cnt, conf, local;
	struct rk_time time;

	prefetch = get_int();
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

		case comm_to:
		case footn_to:
			invalidate_text_stat(get_int());
			break;

		default:
			get_int();
			break;
		}
	}
	get_accept('}');
	get_eat('\n');
}
