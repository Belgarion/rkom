
#include <sys/types.h>

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "rkom_proto.h"
#include "backend.h"

static void async_new_text(void);

struct mesg {
	struct mesg *next;
	int type;
	int conf;
	int pers;
	char *msg;
	char *msg2;
};
static struct mesg *pole;

static void
putinq(struct mesg *m)
{
	struct mesg *w = pole;

	if (w == 0)
		pole = m;
	else {
		while (w->next)
			w = w->next;
		w->next = m;
	}
}

/*
 * Handle a async message. Put it on a queue; then leave it to
 * async_handler() to do the rest.
 */
void
async(int level)
{
	struct mesg *m;
	int narg, type;

	narg = get_int();
	type = get_int();

	switch (type) {
	case 15: /* New text created */
		async_new_text();
		m = calloc(sizeof(struct mesg), 1);
		m->type = type;
		m->msg = "";
		putinq(m);
		break;

	case 12: /* async-send-message */
		m = calloc(sizeof(struct mesg), 1);
		m->type = type; 
		m->conf = get_int();
		m->pers = get_int();
		m->msg = get_string();
		get_eat('\n');
		putinq(m);
		break;
	case 9: /* async-login */
	case 13: /* async-logout */
		m = calloc(sizeof(struct mesg), 1);
		m->type = type;
		m->pers = get_int();
		m->conf = get_int();
		get_eat('\n');
		putinq(m);
		break;

	case 5: /* async-new-name */
		m = malloc(sizeof(struct mesg));
		m->type = type;
		m->pers = get_int();
		m->msg = get_string();
		m->msg2 = get_string();
		get_eat('\n');
		putinq(m);
		break;

	case 16: /* async-new-recipient */
	case 17: /* async-sub-recipient */
		reread_text_stat_bg(get_int());
		reread_conf_stat_bg(get_int());
		get_int();
		get_accept('\n');
		break;

	case 8: /* async-leave-conf */
	case 14: /* async-deleted-text */
	case 18: /* async-new-membership */
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
rk_async_server(void)
{
	struct mesg *m;
	struct rk_async *ra;
	int size;

	if (pole) {
		size = sizeof(struct rk_async);
		if (pole->type == 12)
			size += strlen(pole->msg) + 1;
		if (pole->type == 5)
			size += strlen(pole->msg) + strlen(pole->msg2) + 2;
		ra = calloc(size, 2);
		ra->ra_type = pole->type;
		ra->ra_conf = pole->conf;
		ra->ra_sender = pole->pers;
		ra->ra_message = ra->ra_message2 = "";
		if (pole->type == 12 || pole->type == 5) {
			ra->ra_message = (void *)&ra[1];
			strcpy(ra->ra_message, pole->msg);
		}
		if (pole->type == 5) {
			ra->ra_message2 =
			    ra->ra_message + strlen(pole->msg) + 1;
			strcpy(ra->ra_message2, pole->msg2);
		} else
			ra->ra_message2 = "";
		if (pole->type == 12 || pole->type == 5)
			free(pole->msg);
		if (pole->type == 5)
			free(pole->msg2);
		m = pole;
		pole = pole->next;
		free(m);
	} else {
		ra = calloc(sizeof(struct rk_async), 1);
		ra->ra_message = "";
		ra->ra_message2 = "";
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

		case bcc_recpt:
		case cc_recpt:
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
			reread_text_stat_bg(get_int());
			break;

		default:
			get_int();
			break;
		}
	}
	get_accept('}');
	cnt = get_int(); /* aux-info, throw it away */
	if (cnt) { 
		struct rk_time t;
		char *c;
		get_accept('{');
		for (i = 0; i < cnt; i++) {
			get_int(); /* aux-no */
			get_int(); /* tag */ 
			get_int(); /* creator */
			read_in_time(&t); /* created-at */
			get_int(); /* flags */
			get_int(); /* inherit-limit */
			c = get_string(); /* data */
			if (*c)
				free(c);
		}
		get_accept('}');
	} else
		get_accept('*');
	get_accept('\n');
}
