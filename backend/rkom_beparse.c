
#include <sys/types.h>
#include <sys/uio.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "rkom_proto.h"
#include "backend.h"

int myuid;

struct rk_time *
rk_time_server(void)
{
	struct rk_time *ret;

	ret = calloc(sizeof(struct rk_time), 1);
	send_reply("35\n");
	read_in_time(ret);
	get_accept('\n');
	return ret;
}

void
rk_alive_server(void)
{
	send_reply("82\n");
	get_accept('\n');
}

int32_t
rk_whatido_server(char *args)
{
	int i;
	char *buf;

	buf = alloca(strlen(args));
	sprintf(buf, "4 %ldH%s\n", (long)strlen(args), args);
	i = send_reply(buf);
	get_eat('\n');
	return i;
}


struct rk_confinfo_retval *
rk_matchconf_server(char *name, u_int8_t flags)
{
	struct rk_confinfo_retval *retval;
	int antal, i, p, k;
	char *buf;

	k = (flags & MATCHCONF_CONF) == MATCHCONF_CONF;
	p = (flags & MATCHCONF_PERSON) == MATCHCONF_PERSON;
	buf = alloca(strlen(name) + 30);
	sprintf(buf, "76 %ldH%s %d %d\n", (long)strlen(name), name, p, k);

	send_reply(buf);

	antal = get_int();
	retval = calloc(sizeof(struct rk_confinfo_retval) + 
	    antal * sizeof(struct rk_confinfo), 1);
	if (antal == 0) {
		get_eat('\n');
	} else {
		retval->rcr_ci.rcr_ci_val = (struct rk_confinfo *)&retval[1];
		retval->rcr_ci.rcr_ci_len = antal;
		get_accept('{');
		for (i = 0; i < antal; i++) {
			struct rk_confinfo *rc = &retval->rcr_ci.rcr_ci_val[i];
			rc->rc_name = get_string();
			rc->rc_type = get_int();
			rc->rc_conf_no = get_int();
		}
		get_accept('}');
		get_accept('\n');
	}
	return retval;
}

int32_t
rk_login_server(u_int32_t userid, char *passwd)
{
	char *buf;
	int i;

	buf = alloca(strlen(passwd) + 30);
	sprintf(buf, "62 %d %ldH%s 0\n", userid, (long)strlen(passwd), passwd);
	if (send_reply(buf)) {
		i = get_int();
	} else {
		myuid = userid;
		i = 0;
	}
	get_eat('\n');
	return i;
}

struct rk_unreadconfval *
rk_unreadconf_server(u_int32_t uid)
{
	struct rk_unreadconfval *ure;
	int i, nconfs;
	char buf[15];

	sprintf(buf, "52 %d\n", uid);
	if ((i = send_reply(buf))) {
		i = get_int();
		get_eat('\n');
		ure = calloc(sizeof(struct rk_unreadconfval), 1);
		return ure;
	}
	nconfs = get_int();
	ure = calloc(sizeof(struct rk_unreadconfval) +
	    nconfs * sizeof(u_int32_t), 1);
	ure->ru_confs.ru_confs_val = (u_int32_t *)&ure[1];
	ure->ru_confs.ru_confs_len = nconfs;
	if (nconfs == 0) {
		get_eat('\n');
	} else {
		u_int32_t *val = ure->ru_confs.ru_confs_val;

		get_accept('{');
		for (i = 0; i < nconfs; i++)
			val[i] = get_int();
		get_accept('}');
		get_accept('\n');
	}
	return ure;
}

struct rk_conference *
rk_confinfo_server(u_int32_t mid)
{
	struct rk_conference *ret, *conf;

	ret = calloc(sizeof(struct rk_conference), 1);
	ret->rc_name = "";
	ret->rc_retval = get_conf_stat(mid, &conf);
	if (ret->rc_retval)
		return ret;
	bcopy(conf, ret, sizeof(struct rk_conference));
	return ret;
}

struct rk_person *
rk_persinfo_server(u_int32_t uid)
{
	struct rk_person *ret, *r2; 

	ret = calloc(sizeof(struct rk_person), 1);
	ret->rp_retval = get_pers_stat(uid, &r2);
	if (ret->rp_retval)
		return ret;
	bcopy(r2, ret, sizeof(struct rk_person));
	ret->rp_retval = 0;
	return ret;
}               

struct rk_membership *
rk_membership_server(u_int32_t uid, u_int32_t mid)
{
	struct rk_membership *m, *ret;

	ret = calloc(sizeof(struct rk_membership), 1);
	ret->rm_retval = get_membership(uid, mid, &m);
	if (ret->rm_retval)
		return ret;
	bcopy(m, ret, sizeof(struct rk_membership));
	ret->rm_retval = 0;
	return ret;
}

struct rk_dynamic_session_info_retval *
rk_vilka_server(u_int32_t secs, u_int32_t flags)
{
	struct rk_dynamic_session_info_retval *pp;
	struct rk_dynamic_session_info *ppp;
	char buf[50];
	int antal, i;

	sprintf(buf, "83 %d %d %d\n", (flags & WHO_VISIBLE) != 0, 
	    (flags & WHO_INVISIBLE) != 0, secs);
	send_reply(buf);

	antal = get_int();
	pp = calloc(sizeof(struct rk_dynamic_session_info_retval) +
	    sizeof(struct rk_dynamic_session_info) * antal, 1);
	pp->rdv_rds.rdv_rds_val = (struct rk_dynamic_session_info *)&pp[1];
	pp->rdv_rds.rdv_rds_len = antal;

	if (antal == 0) {
		get_eat('\n');
		return pp;
	}
	ppp = pp->rdv_rds.rdv_rds_val;
	get_accept('{');
	for (i = 0; i < antal; i++) {
		ppp[i].rds_session = get_int();
		ppp[i].rds_person = get_int();
		ppp[i].rds_conf = get_int();
		ppp[i].rds_idletime = get_int();
		ppp[i].rds_flags = get_int();
		ppp[i].rds_doing = get_string();
	}
	get_accept('}');
	get_accept('\n');
	return pp;
}

char *
rk_client_name_server(u_int32_t vers)
{
	char *ret;
	char buf[50];

	sprintf(buf, "70 %d\n", vers);
	if (send_reply(buf)) {
		get_eat('\n');
		return "";
	}
	ret = get_string();
	get_eat('\n');
	if (*ret == 0)
		ret = calloc(10,1);
	return ret;
}

char *
rk_client_version_server(u_int32_t vers)
{
	char *ret;
	char buf[50];

	sprintf(buf, "71 %d\n", vers);
	if (send_reply(buf)) {
		get_eat('\n');
		return "";
	}
	ret = get_string();
	get_eat('\n');
	if (*ret == 0)
		ret = calloc(10,1);
	return ret;
}

struct text_stat_store {
	struct text_stat_store *next;
	int nummer;
	struct rk_text_stat *rts;
	char *text;
};

struct text_stat_store *pole;

static struct text_stat_store *
findtxt(int nr)
{
	struct text_stat_store *walker;

	walker = pole;
	while (walker) {
		if (walker->nummer == nr)
			return walker;
		walker = walker->next;
	}
	return 0;
}

char *
rk_gettext_server(u_int32_t nr)
{
	struct text_stat_store *tss;
	char buf[50], *c;
	int i;

	if ((tss = findtxt(nr)) && (tss->text))
		return strdup(tss->text);

	sprintf(buf, "25 %d 0 2000000\n", nr);
	if (send_reply(buf)) {
		if ((i = get_int()))
			printf("Det sket sej: %d\n", i);
		get_eat('\n');
		return "";
	}
	c = get_string();
	get_accept('\n');
	if (tss == 0) {
		tss = calloc(sizeof(*tss), 1);
		tss->nummer = nr;
		tss->next = pole;
		pole = tss;
	}
	tss->text = strdup(c);
	return c;
}

static void
gettext_callback(int err, int arg)
{
	struct text_stat_store *tss;
	char *c;

	if (err) {
		get_eat('\n');
		return;
	}
	c = get_string();
	get_accept('\n');
	if ((tss = findtxt(arg)) == 0) {
		free(c); /* ??? What happened? */
		return;
	}
	tss->text = c;
}


static void
readin_textstat(struct rk_text_stat *ts)
{
	struct rk_misc_info *pmi;
	struct rk_aux_item *rai;
	int len, i;

	read_in_time(&ts->rt_time);
	ts->rt_author = get_int();
	ts->rt_no_of_lines = get_int();
	ts->rt_no_of_chars = get_int();
	ts->rt_no_of_marks = get_int();
	len = ts->rt_misc_info.rt_misc_info_len = get_int();
	if (len) {
		pmi = calloc(sizeof(struct rk_misc_info), len);
		ts->rt_misc_info.rt_misc_info_val = pmi;
		get_accept('{');
		for (i = 0; i < len; i++) {
			pmi[i].rmi_type = get_int();
			if (pmi[i].rmi_type == rec_time ||
			    pmi[i].rmi_type == sentat)
				read_in_time(&pmi[i].rmi_time);
			else
				pmi[i].rmi_numeric = get_int();
		}
		get_accept('}');
	} else
		get_accept('*');

	/* aux-info */
	ts->rt_aux_item.rt_aux_item_len = len = get_int();
	if (len) { 
		rai = calloc(sizeof(struct rk_aux_item), len);
		ts->rt_aux_item.rt_aux_item_val = rai;
		get_accept('{');
		for (i = 0; i < len; i++) {
			rai[i].rai_aux_no = get_int(); /* aux-no */
			rai[i].rai_tag = get_int(); /* tag */ 
			rai[i].rai_creator = get_int(); /* creator */
			read_in_time(&rai[i].rai_created_at); /* created-at */
			rai[i].rai_flags = get_int(); /* flags */
			rai[i].inherit_limit = get_int(); /* inherit-limit */
			rai[i].rai_data = get_string(); /* data */
		}
		get_accept('}');
	} else
		get_accept('*');
	get_accept('\n');
}

struct rk_text_stat *
rk_textstat_server(u_int32_t nr)
{
	struct text_stat_store *tss;
	struct rk_text_stat *ts;
	char buf[30];

	if ((tss = findtxt(nr)) && (tss->rts)) {
back:		ts = malloc(sizeof(*ts));
		bcopy(tss->rts, ts, sizeof(*ts));
		return ts;
	}
	ts = calloc(sizeof(struct rk_text_stat), 1);
	sprintf(buf, "90 %d\n", nr);
	if (send_reply(buf)) {
		ts->rt_retval = get_int();
		get_eat('\n');
		return ts;
	}
	readin_textstat(ts);
	if (tss == 0) {
		tss = calloc(sizeof(*tss), 1);
		tss->nummer = nr;
		tss->next = pole;
		pole = tss;
	}
	if (tss->text == 0) {
		sprintf(buf, "25 %d 0 2000000\n", nr);
		send_callback(buf, nr, gettext_callback);
        }
	tss->rts = ts;
	goto back;

}

static void
reread_text_stat_bg_callback(int err, int arg)
{
	struct text_stat_store *tss;

	tss = findtxt(arg);
	if (err) {
		printf("Couldn't reread text %d: %d\n", arg, err);
		get_eat('\n');
		free(tss->rts);
		tss->rts = 0;
		return;
	}
	readin_textstat(tss->rts);
	return;
}

void
reread_text_stat_bg(int text)
{
	struct text_stat_store *tss;
	struct rk_text_stat *ts;
	char buf[40];

	tss = findtxt(text);
	if (tss == 0) {
		tss = calloc(sizeof(*tss), 1);
		tss->nummer = text;
		tss->next = pole;
		pole = tss;
	}
	if (tss->rts == 0)
		tss->rts = calloc(sizeof(struct rk_text_stat), 1);
	ts = tss->rts;
	if (ts->rt_misc_info.rt_misc_info_len)
		free(ts->rt_misc_info.rt_misc_info_val);
	if (ts->rt_aux_item.rt_aux_item_len)
		free(ts->rt_aux_item.rt_aux_item_val);
	/* XXX loosing rai_data */
	sprintf(buf, "90 %d\n", text);
	send_callback(buf, text, reread_text_stat_bg_callback);
}
	
/*
 * Put a text into a conference.
 */
struct rk_text_retval *
rk_create_text_server(struct rk_text_info *rti)
{
	struct rk_text_retval *rkr;
	struct rk_misc_info *mi;
	struct rk_aux_item_input *raii;
	extern int sockfd;
	char buf[30];
	int i, nmi, nraii;

	sprintf(buf, "86 %ldH", (long)strlen(rti->rti_text));
	send_reply(buf);
	write(sockfd, rti->rti_text, strlen(rti->rti_text));
	nmi = rti->rti_misc.rti_misc_len;
	mi = rti->rti_misc.rti_misc_val;

	sprintf(buf, " %d { ", nmi);
	send_reply(buf);
	for (i = 0; i < nmi; i++) {
		sprintf(buf, "%d %d ", mi[i].rmi_type, mi[i].rmi_numeric);
		send_reply(buf);
	}
	nraii = rti->rti_input.rti_input_len;
	raii = rti->rti_input.rti_input_val;
	sprintf(buf, "} %d { ", nraii);
	send_reply(buf);
	for (i = 0; i < nraii; i++) {
		char *nbuf;

		sprintf(buf, "%d 00000000 %d ", raii[i].raii_tag,
		    raii[i].inherit_limit);
		send_reply(buf);
		nbuf = alloca(strlen(raii[i].raii_data) + 10);
		sprintf(nbuf, "%ldH%s ", (long)strlen(raii[i].raii_data),
		    raii[i].raii_data);
		send_reply(nbuf);
	}

	rkr = calloc(sizeof(struct rk_text_retval), 1);
	if (send_reply("}\n")) {
		rkr->rtr_status = get_int();
		get_eat('\n');
		return rkr;
	}
	rkr->rtr_textnr = get_int();
	get_accept('\n');

	/*
	 * Invalidate status for those texts this is comment to.
	 * Those will be refetched by async messages anyway.
	 */
	for (i = 0; i < nmi; i++)
		if (mi[i].rmi_type == comm_to || mi[i].rmi_type == footn_to)
			reread_text_stat_bg(mi[i].rmi_numeric);
	return rkr;
}

/* Get the marked texts. */
struct rk_mark_retval *
rk_getmarks_server(void)
{
	struct rk_mark_retval *rmr;
	struct rk_marks *rm;
	int cnt, i;

	rmr = calloc(sizeof(*rmr), 1);
	if (send_reply("23\n")) {
		rmr->rmr_retval = get_int();
		get_eat('\n');
		return rmr;
	}
	cnt = get_int();
	if (cnt) {
		rmr = realloc(rmr, sizeof(*rmr) + cnt * sizeof(*rm));
		rmr->rmr_marks.rmr_marks_len = cnt;
		rmr->rmr_marks.rmr_marks_val = (void *)&rmr[1];
		rm = rmr->rmr_marks.rmr_marks_val;
		get_accept('{');
		for (i = 0; i < cnt; i++) {
			rm[i].rm_text = get_int();
			rm[i].rm_type = get_int();
		}
		get_accept('}');
	} else
		get_accept('*');
	get_accept('\n');
	return rmr;
}

/* Mark a text */
int32_t
rk_setmark_server(u_int32_t text, u_int8_t type)
{
	char buf[30];
	int retval = 0;

	sprintf(buf, "72 %d %d\n", text, type);
	if (send_reply(buf))
		retval = get_int();
	get_eat('\n');
	if (retval == 0) {
		struct text_stat_store *tss;

		tss = findtxt(text);
		if (tss && tss->rts)
			tss->rts->rt_no_of_marks++;
	}
	return retval;
}

/* Unmark a text */
int32_t
rk_unmark_server(u_int32_t text)
{
	char buf[30];
	int retval = 0;

	sprintf(buf, "73 %d\n", text);
	if (send_reply(buf))
		retval = get_int();
	get_eat('\n');
	if (retval == 0) {
		struct text_stat_store *tss;

		tss = findtxt(text);
		if (tss && tss->rts)
			tss->rts->rt_no_of_marks--;
	}
	return retval;
}

static char **spole;

static char **
splitup(char *str, char **nyc, int *cnt)
{
	int maxlen, i, num;
	char *c;

	if (spole)
		free(spole);
	spole = malloc(1000);
	c = str;
	maxlen = atoi(str);
	if (maxlen == 0) {
		*cnt = 0;
		return 0;
	}
	while (*c++ != 'H')
		;
	for (i = 0; c < str + maxlen; i++) {
		num = atoi(c);
		while (*c++ != 'H')
			;
		spole = realloc(spole, (i + 1) * sizeof(char *));
		spole[i] = c;
		c += num;
		*c++ = 0;
	}
	*cnt = i;
	*nyc = c;
	return spole;
}

static char *upole;

struct rk_uarea *
rk_get_uarea_server(char *str)
{
	struct rk_person *p;
	struct rk_uarea *ru;
	struct rk_val *rv;
	char buf[30], *c, **vec;
	int i, j;

	if (upole)
		free(upole);
	ru = calloc(sizeof(*ru), 1);
	if (myuid == 0) {
		ru->ru_retval = 6;
		return ru;
	}
	get_pers_stat(myuid, &p);
	if (p->rp_user_area == 0) {
		ru->ru_retval = 8;
		return ru;
	}
	sprintf(buf, "25 %d 0 2000000\n", p->rp_user_area);
	if (send_reply(buf)) {
		if ((i = get_int()))
			ru->ru_retval = i;
		get_eat('\n');  
		return ru;
	}       
	upole = get_string();
	get_accept('\n'); 

	/* Check if there is any entry in the uarea matching our string */
	vec = splitup(upole, &c, &j);
	for (i = 0; i < j; i++)
		if (strcmp(vec[i], str) == 0)
			break;
	if (vec[i] == 0)
		return ru;

	for (j = 0; j < i; j++)
		c = c + atoi(c) + 1;

	vec = splitup(c, &c, &i);
	free(ru);
	ru = calloc(sizeof(*ru) + sizeof(struct rk_val) * i / 2, 1);
	ru->ru_val.ru_val_len = i / 2;
	rv = (void *)&ru[1];
	ru->ru_val.ru_val_val = rv;
	for (j = 0; j < i; j+=2) {
		rv[j/2].rv_var = vec[j];
		rv[j/2].rv_val = vec[j+1];
	}
	return ru;
}


int32_t
rk_set_uarea_server(char *hej, struct rk_uarea *u)
{
	struct rk_val *v;
	struct rk_person *p;
	int i, len, tot, nlen;
	char *narea, *nname, *tmp, *nhdr, *utstr;

	v = u->ru_val.ru_val_val;
	len = u->ru_val.ru_val_len;
	/* First: make a storage string of our new uarea */
	for (i = tot = 0; i < len; i++)
		tot += strlen(v->rv_var) + strlen(v->rv_val);
	tot += len * 10;
	tmp = alloca(tot);
	*tmp = 0;
	narea = alloca(tot + 20);
	for (i = 0; i < len; i++) {
		sprintf(narea, " %ldH%s %ldH%s\n", (long)strlen(v[i].rv_var),
		    v[i].rv_var, (long)strlen(v[i].rv_val), v[i].rv_val);
		strcat(tmp, narea);
	}
	sprintf(narea, "%ldH%s", (long)strlen(tmp), tmp);
	nname = alloca(strlen(hej) + 20);
	sprintf(nname, "%ldH%s", (long)strlen(hej), hej);
	/* Now the new uarea is in narea and its name in nname */
	get_pers_stat(myuid, &p);
	if (p->rp_user_area) {
		char *txt, *odata, *ohdr, *t;
		char buf[50];
		int cnt;

		sprintf(buf, "25 %d 0 2000000\n", p->rp_user_area);
		if (send_reply(buf)) {
			i = get_int();
			get_eat('\n');  
			return i;
		}       
		txt = get_string();
		get_accept('\n');

		cnt = atoi(txt);
		ohdr = index(txt, 'H');
		ohdr++;
		odata = &ohdr[cnt + 1];
		ohdr[cnt] = 0;
		t = strstr(ohdr, nname); /* Search for correct storage */
		if (t == 0) { /* Not found */
			nhdr = alloca(strlen(nname) + strlen(ohdr) + 50);
			sprintf(nhdr, "%ldH%s %s",
			    (long)strlen(ohdr) + (long)strlen(nname) + 1,
			    ohdr, nname);
			t = alloca(strlen(narea) + strlen(odata) + 50);
			sprintf(t, "%s %s", odata, narea);
			narea = t;
		} else { /* Found, do substitution */


		}
		free(txt);
	} else {
		nhdr = alloca(strlen(nname) + 20);
		sprintf(nhdr, "%ldH %s", (long)strlen(nname) + 1, nname);
	}
	nlen = strlen(nhdr) + strlen(narea);
	utstr = alloca(nlen + 50);
	sprintf(utstr, "86 %dH %s %s * { } * { }\n", nlen + 2, nhdr, narea);
printf("Msg: %s\n", utstr);
#if 0
	if (send_reply(d)) {
		i = get_int();
		get_eat('\n');
		return i;
	}
	no = get_int();
	get_accept('\n');
	sprintf(b, "57 %d %d\n", myuid, no);
	if (send_reply(b)) {
		i = get_int();
		get_eat('\n');
		return i;
	}
	get_accept('\n');
#endif
	return 0;
}

int32_t
rk_send_msg_server(u_int32_t dest, char *string)
{
	char *dst;
	int i = 0;

	dst = alloca(strlen(string) + 30);
	sprintf(dst, "53 %d %ldH%s\n", dest, (long)strlen(string), string);
	if (send_reply(dst))
		i = get_int();
	get_eat('\n');
	return i;
}

int32_t 
rk_setpass_server(u_int32_t uid, char *oldpass, char *newpass)
{
	int i, totlen;
	char *buf;

	totlen = strlen(oldpass) + strlen(newpass) + 50;
	buf = alloca(totlen);
	sprintf(buf, "8 %d %ldH%s %ldH%s\n", uid, (long)strlen(oldpass),
	    oldpass, (long)strlen(newpass), newpass);
	if (send_reply(buf)) {
		i = get_int();
		get_eat('\n');
		return i;
	}
	get_accept('\n');
	return 0;
}
