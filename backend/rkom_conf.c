
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rkomsupport.h"
#include "backend.h"

struct membership_store {
	int mid;
	struct membership_store *next;
	struct rk_membership member;
};

struct person_store {
	int uid;
	struct person_store *nextp;
	struct membership_store *next;
	struct rk_person person;
	int nconfs, *confs;
};

static struct person_store *gps;

static struct person_store *
findperson(int uid)
{
	struct person_store *walker;

	walker = gps;
	while (walker) {
		if (walker->uid == uid)
			return walker;
		walker = walker->nextp;
	}
	return 0;
}
/*
 * Return the person struct.
 */
struct rk_person *
rk_persinfo(u_int32_t uid)
{
	struct person_store *pp;
	struct rk_person *p;

	if ((pp = findperson(uid)))
		return &pp->person;

	if (send_reply("49 %d\n", uid)) {
		komerr = get_int();
		get_eat('\n');
		return NULL;
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
	pp->nextp = gps;
	gps = pp;
	return p;
}

struct get_conf_stat_store {
	struct get_conf_stat_store *next;
	int number;
	struct rk_conference confer;
	int mapsz;	/* Number of element in mapping */
	int *map;	/* Array of mapped ints */
};

static struct get_conf_stat_store *gcs;

static struct get_conf_stat_store *
findconf(int conf)
{
	struct get_conf_stat_store *walker;

	if (gcs != 0) {
		walker = gcs;
		while (walker) {
			if (walker->number == conf)
				return walker;
			walker = walker->next;
		}
	}
	return 0;
}

void
newname(int uid)
{
	struct person_store *pp;
	struct get_conf_stat_store *walker;

	pp = findperson(uid);
	if (pp == 0)
		return;
	walker = findconf(uid);
	if (walker == 0)
		return;
	free(pp->person.rp_username);
	pp->person.rp_username = strdup(walker->confer.rc_name);
}

/*
 * Get the local->global table. Store it as an array from 0 and up,
 * this is waste of memory but we do not care right now.
 */
static void
read_lgtable(struct get_conf_stat_store *g)
{
	int top, i, base, cont, cnt, num;

	if (g->mapsz)
		return; /* Done already */

#if 0
	/* It seems like some real conferences are marked as letterboxes.
	 * so the below code seems to do more harm than good. /oj
	 */
	if (g->number != myuid && 
	    g->confer.rc_type & RK_CONF_TYPE_LETTERBOX) 
		return; /* Do not try to read someones letterbox. */
#endif
	/*
	 * First get the highest local text number.
	 */
	if (send_reply("78 %d\n", g->number)) {
		get_eat('\n'); /* Do what??? */
		return;
	}
	get_string();
	get_int();
	top = get_int();
	g->mapsz = top + 50;
	get_int();
	get_accept('\n');

	g->map = calloc(g->mapsz, sizeof(int));
	base = 1;
	for (;;) {
		if (send_reply("103 %d %d 200\n", g->number, base)) {
			get_eat('\n');
			return;	/* XXX ??? */
		}
		get_int();
		base = get_int();
		cont = get_int();
		if (get_int() == 0) {
			/* Sparse block */
			cnt = get_int();
			if (cnt != 0) {
				get_accept('{');
				for (i = 0; i < cnt; i++) {
					num = get_int();
					g->map[num] = get_int();
				}
				get_accept('}');
			} else
				get_accept('*');
		} else {
			/* Dense block */
			num = get_int();
			cnt = get_int();
			get_accept('{');
			for (i = 0; i < cnt; i++)
				g->map[num++] = get_int();
			get_accept('}');
		}
		get_accept('\n');
		if (cont == 0)
			break;
	}
}

static void
readin_conf_stat(struct rk_conference *c)
{
	struct rk_aux_item *rai;
	int i, len;

	c->rc_name = get_string();
	c->rc_type = get_bitfield();
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

	c->rc_aux_item.rc_aux_item_len = len = get_int();
	if (len) {
		rai = calloc(sizeof(struct rk_aux_item), len);
		c->rc_aux_item.rc_aux_item_val = rai;
		get_accept('{');
		for (i = 0; i < len; i++)
			read_in_aux_item(&rai[i]);
		get_accept('}');
	} else
		get_accept('*');
	get_accept('\n');
}

static void
reread_conf_stat_bg_callback(int err, int arg)
{
	struct get_conf_stat_store *walker;

	if (err) {
		printf("reread_conf_stat_bg_callback: error %d\n", err);
		get_eat('\n');
		return;
	}
	walker = findconf(arg);
	if (walker) {
		free(walker->confer.rc_name);
	} else {
		walker = calloc(sizeof(struct get_conf_stat_store), 1);
		walker->number = arg;
		walker->next = gcs;
		gcs = walker;
	}
	readin_conf_stat(&walker->confer);
}

void	
reread_conf_stat_bg(int conf)
{
	char buf[40];

	sprintf(buf, "91 %d\n", conf);
	send_callback(buf, conf, reread_conf_stat_bg_callback);
}

/*
 * Returns the conference struct for a given conference.
 * If it's not in the cache; ask the server for it.
 */
struct rk_conference *
rk_confinfo(u_int32_t conf)
{
	struct get_conf_stat_store *walker;
	struct rk_conference *c;

	/* First, see if we have this conference in the cache */
	if ((walker = findconf(conf)))
		return &walker->confer;

	/* Nope, alloc a new struct and put it into the cache */
	if (send_reply("91 %d\n", conf)) {
		komerr = get_int();
		get_eat('\n');
		return NULL;
	}
	walker = calloc(sizeof(struct get_conf_stat_store), 1);
	walker->number = conf;
	c = &walker->confer;
	readin_conf_stat(c);

	walker->next = gcs;
	gcs = walker;
	return c;
}

static struct membership_store *
findmember(int conf, struct membership_store *mb)
{
	while (mb) {
		if (mb->mid == conf)
			return mb;
		mb = mb->next;
	}
	return 0;
}

struct rk_membership *
rk_membership(u_int32_t uid, u_int32_t conf)
{
	struct membership_store *mb;
	struct rk_membership *m;
	struct person_store *pp;
	int len;

	/* First, force the user into the cache */
	if (rk_persinfo(uid) == NULL)
		return NULL; /* The person do not exist, or something */

	pp = findperson(uid);
	if (pp->next) {
		mb = findmember(conf, pp->next);
		if (mb)
			return &mb->member;
	}

	/* No, we failed cache search. Fetch from server. */
	if (send_reply("98 %d %d\n", uid, conf)) {
		komerr = get_int();
		get_eat('\n');
		return NULL;
	}
	mb = calloc(sizeof(struct membership_store), 1);
	m = &mb->member;
	m->rm_position = get_int();
	read_in_time(&m->rm_last_time_read);
	m->rm_conference = get_int();
	m->rm_priority = get_int();
	m->rm_last_text_read = get_int();
	len = m->rm_read_texts.rm_read_texts_len = get_int();
	if (len) {
		int *txts, j;

		txts = malloc(sizeof(int) * len);
		get_accept('{');
		for (j = 0; j < len; j++)
			txts[j] = get_int();
		get_accept('}');
		m->rm_read_texts.rm_read_texts_val = txts;
	} else
		get_accept('*');
	m->rm_added_by = get_int();
	read_in_time(&m->rm_added_at);
	m->rm_type = get_int();

	mb->mid = m->rm_conference;
	mb->next = pp->next;
	pp->next = mb;
	get_accept('\n');
	return m;
}

static void
delete_membership_internal(int conf, int uid)
{
	struct membership_store *mb, *omb;
	struct person_store *pp;

	if ((pp = findperson(uid)) == 0)
		return; /* Nothing in cache */

	if (pp->nconfs) {
		free(pp->confs);
		pp->nconfs = 0;
	}
	if (pp->next == 0)
		return;

	if (pp->next->mid == conf) {
		mb = pp->next->next;
		free(pp->next);
		pp->next = mb;
		return;
	}
	omb = pp->next;
	mb = pp->next->next;
	while (mb) {
		if (mb->mid == conf) {
			omb->next = mb->next;
			free(mb);
			return;
		}
		omb = mb;
		mb = mb->next;
	}
}

static void
set_last_read_internal(int conf, int local)
{
	struct membership_store *mb;
	struct person_store *pp;

	if ((pp = findperson(myuid)) == 0)
		return; /* Nothing in cache */

	if (pp->next) {
		mb = findmember(conf, pp->next);
		if (mb == 0)
			return;
		mb->member.rm_last_text_read = local;
		if (mb->member.rm_read_texts.rm_read_texts_len)
			free(mb->member.rm_read_texts.rm_read_texts_val);
		mb->member.rm_read_texts.rm_read_texts_len = 0;
	}
}

int32_t
rk_change_conference(u_int32_t conf)
{
	int ret;

	ret = send_reply("2 %d\n", conf);
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
	struct get_conf_stat_store *g;
	int i;

	if ((g = findconf(conf)) == NULL) {
		rk_confinfo(conf);
		if ((g = findconf(conf)) == NULL)
			return 0;
	}
	if (g->map == NULL)
		read_lgtable(g);

	if (local >= g->mapsz)
		return 0;
	for (i = local; i < g->mapsz; i++)
		if (g->map[i])
			return i;
	return 0;
}

void
invalidate_local(struct rk_text_stat *ts)
{
	struct get_conf_stat_store *g;
	struct rk_misc_info *mi = ts->rt_misc_info.rt_misc_info_val;
	int nmi = ts->rt_misc_info.rt_misc_info_len;
	int i, conf, local;

	for (i = 0; i < nmi; i++) {
		if (mi[i].rmi_type != recpt && mi[i].rmi_type != cc_recpt)
			continue;
		conf = mi[i].rmi_numeric;
		if (mi[i+1].rmi_type != loc_no)
			continue;
		i++;
		local = mi[i].rmi_numeric;
		if ((g = findconf(conf)) == NULL)
			continue;
		if (local >= g->mapsz)
			continue;
		g->map[local] = 0;
	}
}

static int
is_read(int conf, int text, int uid)
{
	struct rk_membership *m;
	int i, num, *txts;

	if ((m = rk_membership(uid, conf)) == NULL)
		return 1; /* ??? */
	num = m->rm_read_texts.rm_read_texts_len;
	txts = m->rm_read_texts.rm_read_texts_val;
	for (i = 0; i < num; i++)
		if (txts[i] == text)
			return 1;
	return 0;
}

/*
 * Check if text nr is marked read in conference conf.
 */
int
rk_local_is_read(u_int32_t conf, u_int32_t localno)
{
	struct rk_membership *m;
	int i, num, *txts;

	if ((m = rk_membership(myuid, conf)) == NULL)
		return 1; /* not member */

	num = m->rm_read_texts.rm_read_texts_len;
	txts = m->rm_read_texts.rm_read_texts_val;
	for (i = 0; i < num; i++)
		if (txts[i] == localno)
			return 1;
	return 0;
}

u_int32_t
rk_next_unread(u_int32_t conf, u_int32_t uid)
{
	struct rk_conference *c;
	struct rk_membership *m;
	int highest, last;

	if ((c = rk_confinfo(conf)) == NULL)
		return 0;
	highest = c->rc_first_local_no + c->rc_no_of_texts - 1;
	if ((m = rk_membership(uid, conf)) == NULL)
		return 0;

back:	last = m->rm_last_text_read;
	if (highest <= last)
		return 0;
	last = next_local(conf, last + 1);
	if (last == 0) {
		rk_mark_read(conf, m->rm_last_text_read + 1);
		goto back;
	}
	if (is_read(conf, last, uid)) {
		m->rm_last_text_read = last;
		goto back;
	}

	if (last > highest)
		return 0;
	else
		return last;
}

u_int32_t
rk_local_to_global(u_int32_t conf, u_int32_t local)
{
	struct get_conf_stat_store *g;

	if ((g = findconf(conf)) == NULL) {
		rk_confinfo(conf);
		if ((g = findconf(conf)) == NULL)
			return 0;
	}
	if (g->map == NULL)
		read_lgtable(g);

	if (local >= g->mapsz)
		return 0;
	return g->map[local];
}

void
conf_set_high_local(int conf, int local, int global)
{
	struct get_conf_stat_store *walker;
	int tt;

	/* First, see if we have this conference in the cache */
	if (gcs == 0)
		return;
	walker = gcs;
	while (walker) {
		if (walker->number == conf) {
			tt = local - walker->confer.rc_first_local_no+1;
			if (walker->confer.rc_no_of_texts < tt)
				walker->confer.rc_no_of_texts = tt;
			if (walker->map == NULL)
				return;

			if (walker->mapsz <= local) {
				int *tpt = calloc(local+50, sizeof(int));
				memcpy(tpt, walker->map, walker->mapsz *
				    sizeof(int));
				free(walker->map);
				walker->map = tpt;
				walker->mapsz = local+50;
			}
			walker->map[local] = global;
			return;
		}
		walker = walker->next;
	}
}

static void
cleanup_read(struct rk_membership *m)
{
	int local, len, nlen, *txts, *ntxts, i;

	local = m->rm_last_text_read;
	len = m->rm_read_texts.rm_read_texts_len;
	txts = m->rm_read_texts.rm_read_texts_val;
	if (len == 0)
		return;
	for (i = 0; i < len; i++)
		if (txts[i] == local + 1) {
			local++;
			i = 0;
		}
	ntxts = malloc(sizeof(int) * len);
	nlen = 0;
	for (i = 0; i < len; i++)
		if (txts[i] > local)
			ntxts[nlen++] = txts[i];
	free(txts);
	m->rm_read_texts.rm_read_texts_val = ntxts;
	m->rm_read_texts.rm_read_texts_len = nlen;
	m->rm_last_text_read = local;
}

static void
rk_mark_read_server_callback(int err, int arg)
{
	if (err)
		get_eat('\n');
	else
		get_accept('\n');
}

int32_t
rk_mark_read(u_int32_t conf, u_int32_t local)
{
	struct rk_membership *m;
	char buf[50];
	int i, num, *txts;

	if (is_read(conf, local, myuid))
		return 0;
	if ((m = rk_membership(myuid, conf)) != NULL) {
		if (m->rm_last_text_read + 1 == local) {
			m->rm_last_text_read = local;
			cleanup_read(m);
		} else {
			txts = m->rm_read_texts.rm_read_texts_val;
			num = m->rm_read_texts.rm_read_texts_len;
			if (num == 0)
				txts = NULL;
			for (i = 0; i < num; i++)
				if (txts[i] == local)
					break;
			if (i == num) {
				num++;
				txts = realloc(txts, sizeof(int) * num);
				txts[num-1] = local;
				m->rm_read_texts.rm_read_texts_val = txts;
				m->rm_read_texts.rm_read_texts_len = num;
			}
		}
	}
	i = 0;
	sprintf(buf, "27 %d 1 { %d }\n", conf, local);
	send_callback(buf, 0, rk_mark_read_server_callback);
	return 0;
};

int32_t
rk_set_last_read(u_int32_t conf, u_int32_t local)
{

	if (send_reply("77 %d %d\n", conf, local)) {
		int ret = get_int();
		get_eat('\n');
		return ret;
	}
	get_eat('\n');
	set_last_read_internal(conf, local);
	return 0;
}

int32_t 
rk_add_member(u_int32_t conf, u_int32_t uid, u_int8_t prio,
    u_int16_t where, u_int32_t flags)
{
	int ret;

	ret = send_reply("100 %d %d %d %d %d\n", conf, uid, prio, where, flags);
	if (ret) {
		ret = get_int();
		get_eat('\n');
		return ret;
	}
	get_accept('\n');
	delete_membership_internal(conf, uid);
	return ret;
}

int32_t
rk_sub_member(u_int32_t conf, u_int32_t uid)
{
	int ret;

	ret = send_reply("15 %d %d\n", conf, uid);
	if (ret) {
		ret = get_int();
		get_eat('\n');
		return ret;
	}
	get_accept('\n');
	delete_membership_internal(conf, uid);
	return ret;
}

/*
 * XXX - should remember the membership structs also.
 */
struct rk_memberconflist *
rk_memberconf(u_int32_t uid)
{
	static struct rk_memberconflist rkm;
	struct person_store *ps;
	int i;

	ps = findperson(uid);
	if (ps == 0) {
		if (rk_persinfo(uid) == NULL)
			return NULL;
		ps = findperson(uid); /* Cannot fail here */
		if (ps == 0)
			printf("EEEK! Person %d finns inte!\n", uid);
	}
	if (ps->nconfs == 0) {
		if (send_reply("46 %d 0 65535 0\n", uid)) {
			komerr = get_int();
			get_eat('\n');
			return NULL;
		}
		ps->nconfs = get_int();
		if (ps->nconfs) {
			ps->confs = malloc(sizeof(int) * ps->nconfs);
			get_accept('{');
			for (i = 0; i < ps->nconfs; i++) {
				struct rk_time t;

				read_in_time(&t);
				ps->confs[i] = get_int();
				get_int();get_int();get_int();get_accept('*');
			}
			get_accept('}');
		} else
			get_accept('*');
		get_accept('\n');
	}
	rkm.rm_confs.rm_confs_len = ps->nconfs;
	rkm.rm_confs.rm_confs_val = ps->confs;
	return &rkm;
}

/*
 * Delete all cached membership records.
 */
void
rk_sync(void)
{
	struct membership_store *m, *nm;
	struct person_store *walker;

	walker = gps;
	while (walker) {
		if (walker->next) {
			m = walker->next;
			while (m) {
				if (m->member.rm_read_texts.rm_read_texts_len)
					free(m->member.rm_read_texts.rm_read_texts_val);
				nm = m->next;
				free(m);
				m = nm;
			}
			walker->next = NULL;
		}
		walker = walker->nextp;
	}
}

int32_t
rk_create_conf(char *name, u_int32_t btype)
{
	char *type;
	int i;

	type = bitfield2str(btype);
	if (send_reply("88 %ldH%s %s 0 { }\n",
	    (long)strlen(name), name, &type[28])) {
		i = get_int();
		get_eat('\n');
		return -i;
	}
	i = get_int();
	get_accept('\n');
	return i;
}

int32_t
rk_create_person(char *name, char *passwd, u_int32_t btype)
{
	int i;

	if (send_reply("89 %ldH%s %ldH%s 00000000 0 { }\n",
	    (long)strlen(name), name, (long)strlen(passwd), passwd)) {
		i = get_int();
		get_eat('\n');
		return -i;
	}
	i = get_int();
	get_accept('\n');
	return i;
}

int32_t 
rk_delete_conf(u_int32_t conf)
{
	int i;

	if (send_reply("11 %d\n", conf)) {
		i = get_int();
		get_eat('\n');
		return i;
	}
	get_accept('\n');
	return 0;
}

int32_t
rk_modify_conf_info(struct rk_modifyconfinfo *rkm)
{
	struct rk_aux_item_input *raii;
	int i, nr;

	/* Send request */
	send_reply("93 %d ", rkm->rkm_conf);

	/* Send delete items */
	nr = rkm->rkm_delete.rkm_delete_len;
	send_reply(" %d { ", nr);
	for (i = 0; i < nr; i++) {
		send_reply("%d ", rkm->rkm_delete.rkm_delete_val[i]);
	}

	/* Send add items */
	nr = rkm->rkm_add.rkm_add_len;
	raii = rkm->rkm_add.rkm_add_val;
	send_reply("} %d { ", nr);
	for (i = 0; i < nr; i++) {
		send_reply("%d 00000000 %d ", raii[i].raii_tag,
		    raii[i].inherit_limit);
		send_reply("%ldH%s ", (long)strlen(raii[i].raii_data),
		    raii[i].raii_data);
	}
	i = 0;
	if (send_reply("}\n")) {
		i = get_int();
		get_eat('\n');
		return i;
	}
	get_accept('\n');
	reread_conf_stat_bg(rkm->rkm_conf);
	return 0;
}
