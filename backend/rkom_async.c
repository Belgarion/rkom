
#include <sys/types.h>

#include <stdio.h>
#ifdef SOLARIS
#undef _XPG4_2
#endif
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "rkomsupport.h"
#include "rkom_proto.h"
#include "backend.h"


struct mesg {
	struct rk_async ra;
	struct mesg *next;
};
static struct mesg *pole;

static void async_new_text(struct mesg *);

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
 * Handle an async message. Put it on a queue; then leave it to
 * async_handler() to do the rest.
 */
void
async(int level)
{
	struct mesg *m;
	int narg, type, tmp;

	narg = get_int();
	type = get_int();
	m = calloc(sizeof(struct mesg), 1);
	m->ra.ra_message = m->ra.ra_message2 = "";
	m->ra.ra_type = type;

	switch (type) {
	case 15: /* New text created */
		async_new_text(m);
		putinq(m);
		break;

	case 12: /* async-send-message */
		m->ra.ra_conf = get_int();
		m->ra.ra_pers = get_int();
		m->ra.ra_message = get_string();
		get_accept('\n');
		putinq(m);
		break;

	case 9: /* async-login */
	case 13: /* async-logout */
		m->ra.ra_pers = get_int();
		m->ra.ra_conf = get_int();
		get_accept('\n');
		putinq(m);
		break;

	case 5: /* async-new-name */
		tmp = m->ra.ra_pers = get_int();
		m->ra.ra_message = get_string();
		m->ra.ra_message2 = get_string();
		get_accept('\n');
		putinq(m);
		reread_conf_stat_bg(tmp);
		newname(tmp);
		break;

	case 16: /* async-new-recipient */
	case 17: /* async-sub-recipient */
		reread_text_stat_bg(get_int());
		reread_conf_stat_bg(get_int());
		get_int();
		get_accept('\n');
		free(m);
		break;

	case 8: /* async-leave-conf */
		m->ra.ra_conf = get_int();
		get_accept('\n');
		putinq(m);
		break;

	case 14: /* async-deleted-text */
		m->ra.ra_text = get_int();
		reread_text_stat_bg(m->ra.ra_text);
		get_eat('\n');
		putinq(m);
		break;

	case 18: /* async-new-membership */
		m->ra.ra_pers = get_int();
		m->ra.ra_conf = get_int();
		get_accept('\n');
		putinq(m);
		break;

	default:
		printf("BG: Unknown async %d\n", type);

	case 7:	/* Save database */
		get_eat('\n');
		free(m);
		return;
	}
	if (level == 1)
		async_handle();
}

void
async_handle()
{
	extern int fepid;

	if (pole)
		kill(fepid, SIGIO);
}

static struct mesg *freepole = 0;

struct rk_async *
rk_async_server(void)
{
	struct rk_async *ra;

	if (freepole) {
		if (*freepole->ra.ra_message)
			free(freepole->ra.ra_message);
		if (*freepole->ra.ra_message2)
			free(freepole->ra.ra_message2);
		free(freepole);
		freepole = 0;
	}
	ra = calloc(sizeof(struct rk_async), 1);
	if (pole) {
		bcopy(pole, ra, sizeof(struct rk_async));
		freepole = pole;
		pole = pole->next;
	} else
		ra->ra_message = ra->ra_message2 = "";
	return ra;
}

void
async_new_text(struct mesg *m)
{
	int i, type, cnt, conf, local;
	struct rk_time time;

	m->ra.ra_text = get_int();
	reread_text_stat_bg(m->ra.ra_text);
	read_in_time(&time);
	m->ra.ra_pers = get_int();
	get_int();get_int();get_int();
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
