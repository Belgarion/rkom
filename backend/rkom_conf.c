
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rkom_proto.h"
#include "exported.h"
#include "backend.h"

struct person_store {
	struct person_store *next;
	int uid;
	struct rk_person person;
};

static struct person_store *gps;
/*
 * Return the person struct.
 */
int
get_pers_stat(int uid, struct rk_person **person)
{
	struct person_store *walker, *pp;
	struct rk_person *p;
	int i;
	char buf[20];

	walker = gps;
	while (walker) {
		if (walker->uid == uid) {
			*person = &walker->person;
			return 0;
		}
		walker = walker->next;
	}

	sprintf(buf, "49 %d\n", uid);
	if (send_reply(buf)) {
		i = get_int();
		get_eat('\n');
		return i;
	}

	pp = calloc(sizeof(struct person_store), 1);
	pp->uid = uid;
	p = &pp->person;
	p->rp_username = get_string();
	p->rp_privileges = get_int();
	p->rp_flags = get_int();
	read_in_time(&p->rp_last_login);
	p->rp_user_area = get_int();
	p->rp_total_time_present = get_int();
	p->rp_sessions = get_int();
	p->rp_created_lines = get_int();
	p->rp_created_bytes = get_int();
	p->rp_read_texts = get_int();
	p->rp_no_of_text_fetches = get_int();
	p->rp_created_persons = get_int();
	p->rp_created_confs = get_int();
	p->rp_first_created_local_no = get_int();
	p->rp_no_of_created_texts = get_int();
	p->rp_no_of_marks = get_int();
	p->rp_no_of_confs = get_int();
	get_accept('\n');
	pp->next = gps;
	gps = pp;
	*person = p;
	return 0;
}

#if 0
void
conf_delete(char *name)
{
	struct conf *ow, *w;

	if (person == 0)
		return;

	if (strcmp(person->p_name, name) == 0) {
		w = person;
		person = person->p_next;
		free(w->p_name);
		free(w);
		return;
	}
	w = person->p_next;
	ow = person;
	while (w) {
		if (strcmp(w->p_name, name) == 0) {
			ow->p_next = w->p_next;
			free(w->p_name);
			free(w);
			return;
		}
		ow = w;
		w = w->p_next;
	}
}

#endif

struct get_conf_stat_store {
	struct get_conf_stat_store *next;
	int number;
	struct rk_conference confer;
};

static struct get_conf_stat_store *gcs;

/*
 * Returns the conference struct for a given conference.
 * If it's not in the cache; ask the server for it.
 */
int
get_conf_stat(int conf, struct rk_conference **confer)
{
	struct get_conf_stat_store *walker;
	struct rk_conference *c;
	int i;
	char buf[20];

	/* First, see if we have this conference in the cache */
	if (gcs != 0) {
		walker = gcs;
		while (walker) {
			if (walker->number == conf) {
				*confer = &walker->confer;
				return 0;
			}
			walker = walker->next;
		}
	}

	/* Nope, alloc a new struct and put it into the cache */
	sprintf(buf, "91 %d\n", conf);
	if (send_reply(buf)) {
		i = get_int();
		get_eat('\n');
		return i;
        }
	walker = calloc(sizeof(struct get_conf_stat_store), 1);
	walker->number = conf;
	c = &walker->confer;

	c->rc_name = get_string();
	c->rc_type = get_int();
	read_in_time(&c->rc_creation_time);
	read_in_time(&c->rc_last_written);
	c->rc_creator = get_int();
	c->rc_presentation = get_int();
	c->rc_supervisor = get_int();
	c->rc_permitted_submitters = get_int();
	c->rc_super_conf = get_int();
	c->rc_msg_of_day = get_int();
	c->rc_nice = get_int();
	c->rc_keep_commented = get_int();
	c->rc_no_of_members = get_int();
	c->rc_first_local_no = get_int();
	c->rc_no_of_texts = get_int();
	c->rc_expire = get_int();
#if 0
	cnt = get_int();
	if (cnt) {
		get_accept('{');
		c->aux_items = calloc((cnt+1), sizeof(struct aux_item));
		for (i = 0; i < cnt; i++)
			read_in_aux_item(&c->aux_items[i]);
		get_accept('}');
	} else {
		c->aux_items = 0;
		get_accept('*');
	}
	get_accept('\n');
#else
	get_eat('\n');
#endif
	walker->next = gcs;
	gcs = walker;
	*confer = c;

	return 0;
}

struct get_membership_store {
	struct get_membership_store *next;
	int number;
	struct rk_membership member;
};

static struct get_membership_store *gms;
static struct rk_membership members;

int
get_membership(int uid, int conf, struct rk_membership **member)
{
	struct get_membership_store *walker;
	struct rk_membership mb;
	struct rk_membership *m = &mb;
	int i, cnt;
	char buf[30];

	*member = 0;
	/* Only do this if I am the one in charge */
	if (uid == myuid) {
		if (gms != 0) {
			walker = gms;
			while (walker) {
				if (walker->number == conf) {
					*member = &walker->member;
					return -1;
				}
				walker = walker->next;
			}
			return -1; /* Didn't exist */
		}
		/* No, we failed cache search. Fetch from server. */
		sprintf(buf, "99 %d 0 65535 0\n", uid);
		if (send_reply(buf)) {
			i = get_int();
			get_eat('\n');
			return i;
		}
		cnt = get_int();
		get_accept('{');
		for (i = 0; i < cnt; i++) {
			m->rm_position = get_int();
			read_in_time(&m->rm_last_time_read);
			m->rm_conference = get_int();
			m->rm_priority = get_int();
			m->rm_last_text_read = get_int();
			m->rm_read_texts.rm_read_texts_len = get_int();
			get_accept('*');
			m->rm_added_by = get_int();
			read_in_time(&m->rm_added_at);
			m->rm_type = get_int();

			walker = malloc(sizeof(struct get_membership_store));
			walker->number = m->rm_conference;
			walker->member = mb;
			walker->next = gms;
			gms = walker;
			if (m->rm_conference == conf)
				*member = &walker->member;
		}
		get_accept('}');
		get_eat('\n');
		return (!*member);
	}
	sprintf(buf, "99 %d 0 65535 0\n", uid);
	if (send_reply(buf)) {
		i = get_int();
		get_eat('\n');
		return i;
	}
	cnt = get_int();
	get_accept('{');
	for (i = 0; i < cnt; i++) {
		m->rm_position = get_int();
		read_in_time(&m->rm_last_time_read);
		m->rm_conference = get_int();
		m->rm_priority = get_int();
		m->rm_last_text_read = get_int();
		m->rm_read_texts.rm_read_texts_len = get_int();
		get_accept('*');
		m->rm_added_by = get_int();
		read_in_time(&m->rm_added_at);
		m->rm_type = get_int();
		if (m->rm_conference == conf) {
			members = mb;
			*member = &members;
			get_eat('\n');
			return 0;
		}
	}
	get_accept('}');
	get_eat('\n');
	return -1;
}

static void
delete_membership_internal(void)
{
	struct get_membership_store *walker, *owalker;

	if (gms == 0)
		return;

	walker = gms;
	while (walker) {
		owalker = walker->next;
		free(walker);
		walker = owalker;
	}
	gms = 0;
}

static void
set_last_read_internal(int conf, int local)
{
	struct get_membership_store *walker;

	walker = gms;
	while (walker) {
		if (walker->number == conf) {
			walker->member.rm_last_text_read = local;
			return;
		}
		walker = walker->next;
	}
}

#if 0
static int *confs = 0;

int *
get_unread_confs(int uid)
{
	int i, nconfs;
	char buf[20];

	sprintf(buf, "52 %d\n", uid);
	if (send_reply(buf)) {
		if ((i = get_int()))
			printf("Get_unread_confs sket sej: %s\n", error(i));
		get_eat('\n');
		return 0;
	}
	nconfs = get_int();
	if (nconfs == 0) {
		get_eat('\n');
		return 0;
	}
	if (confs)
		free(confs);
	confs = calloc(sizeof(int), nconfs + 1);
	get_accept('{');
	for (i = 0; i < nconfs; i++)
		confs[i] = get_int();
	get_accept('}');
	get_accept('\n');
	return confs;
}

static int *texts;

int *
get_map(int conf, int first, int otexts)
{
	char buf[30];
	int gotten, off = 0, ntexts, i;

	if (texts)
		free(texts);

	texts = calloc(sizeof(int), otexts);
	sprintf(buf, "34 %d %d %d\n", conf, first, otexts);

	if (send_reply(buf)) {
		if ((i = get_int()))
			printf("Det sket sej: %s\n", error(i));
		get_eat('\n');
		return 0;
	}
	gotten = get_int();
	if (gotten != first)
		off = gotten - first;

	ntexts = get_int();
	get_accept('{');
	for (i = off; i < (off + ntexts); i++)
		texts[i] = get_int();
	get_accept('}');
	get_accept('\n');
	return texts;
}
#endif

int32_t
rk_change_conference_server(u_int32_t conf)
{
	int ret;
	char buf[20];

	sprintf(buf, "2 %d\n", conf);
	ret = send_reply(buf);
	if (ret) {
		ret = get_int();
		get_eat('\n');
	} else
		get_accept('\n');
	return ret;
}

static int
next_local(int conf, int local)
{
	int ret;
	char buf[30];

	sprintf(buf, "103 %d %d 1\n", conf, local);
	ret = send_reply(buf);
	if (ret) {
		get_eat('\n');
		return 0;
	}
	get_int();get_int();get_int();get_int();
	ret = get_int();
	get_eat('\n');
	return ret;
}

u_int32_t
rk_next_unread_server(u_int32_t conf, u_int32_t uid)
{
	struct rk_conference *c;
	struct rk_membership *m;
	int highest, last;

	get_conf_stat(conf, &c);
	highest = c->rc_first_local_no + c->rc_no_of_texts - 1;
	get_membership(uid, conf, &m);
	last = m->rm_last_text_read;

	last = next_local(conf, last + 1);
	if (last == 0)
		return 0;

	if (last > highest)
		return 0;
	else
		return last;
}

u_int32_t
rk_local_to_global_server(u_int32_t conf, u_int32_t local)
{
	int ret, junk;
	char buf[30];

	sprintf(buf, "103 %d %d 1\n", conf, local);
	ret = send_reply(buf);
	if (ret) {
		ret = get_int();
		printf("local_to_global sket sej: %d\n", (ret));
		get_eat('\n');
		return 0;
	}
	junk = get_int(); /* first */
	junk = get_int(); /* last */
	junk = get_int(); /* later texts */
	junk = get_int(); /* sparse/dense, must be dense */
	junk = get_int(); /* must be == local */
	if (junk != local) {
		get_eat('\n');
		return 0;
	}
	junk = get_int(); /* Numbers of texts in array, must be 1 */
	get_accept('{');
	ret = get_int(); /* Our global number */
	get_accept('}');
	get_accept('\n');
	return ret;
}

void
conf_set_high_local(int conf, int local)
{
	struct get_conf_stat_store *walker;

	/* First, see if we have this conference in the cache */
	if (gcs != 0) {
		walker = gcs;
		while (walker) {
			if (walker->number == conf) {
				walker->confer.rc_no_of_texts++;
				return;
			}
			walker = walker->next;
		}
		printf("Internal strange... conf %d not in cache.\n", conf);
	}
}

int32_t
rk_mark_read_server(u_int32_t conf, u_int32_t local)
{
	char buf[50];
	int i = 0;

	set_last_read_internal(conf, local);
	sprintf(buf, "27 %d 1 { %d }\n", conf, local);
	if (send_reply(buf))
		i = get_int();
	get_eat('\n');
	return i;
};

int32_t
rk_set_last_read_server(u_int32_t conf, u_int32_t local)
{
	char buf[20];

	sprintf(buf, "77 %d %d\n", conf, local);
	if (send_reply(buf)) {
		int ret = get_int();
		get_eat('\n');
		return ret;
	}
	get_eat('\n');
	set_last_read_internal(conf, local);
	return 0;
}

int32_t 
rk_add_member_server(u_int32_t conf, u_int32_t uid, u_int8_t prio,
    u_int16_t where, u_int32_t flags)
{
	int ret;
	char buf[30];

	sprintf(buf, "100 %d %d %d %d %d\n", conf, uid, prio, where, flags);
	ret = send_reply(buf);
	if (ret) {
		ret = get_int();
		get_eat('\n');
		return ret;
	}
	get_accept('\n');
	delete_membership_internal();
	return ret;
}
