
#include <sys/types.h>
#include <sys/uio.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "rkom_proto.h"
#include "backend.h"
#include "exported.h"

int myuid;

struct rk_time *
rk_time_server(int32_t arg)
{
	struct rk_time *ret;

	ret = calloc(sizeof(struct rk_time), 1);
	send_reply("35\n");
	read_in_time(ret);
	get_accept('\n');
	return ret;
}

int32_t
rk_alive_server(int32_t arg)
{
	send_reply("82\n");
	get_accept('\n');
	return 0;
}

int32_t
rk_whatido_server(struct rk_whatidoargs *args)
{
	int i;
	char *buf;

	buf = alloca(strlen(args->rw_whatido));
	sprintf(buf, "4 %ldH%s\n", (long)strlen(args->rw_whatido),
	    args->rw_whatido);
	i = send_reply(buf);
	get_eat('\n');
	return i;
}


struct rk_confinfo_retval *
rk_matchconf_server(struct rk_matchconfargs *args)
{
	struct rk_confinfo_retval *retval;
	int antal, i, p, k;
	char *buf;

	k = (args->rm_flags & MATCHCONF_CONF) == MATCHCONF_CONF;
	p = (args->rm_flags & MATCHCONF_PERSON) == MATCHCONF_PERSON;
	buf = alloca(strlen(args->rm_name) + 30);
	sprintf(buf, "76 %ldH%s %d %d\n",
	    (long)strlen(args->rm_name), args->rm_name, p, k);

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
rk_login_server(struct rk_loginargs *args)
{
	char *buf;
	int i;

	buf = alloca(strlen(args->rk_passwd) + 30);
	sprintf(buf, "62 %d %ldH%s 0\n", args->rk_userid,
	    (long)strlen(args->rk_passwd), args->rk_passwd);
	if (send_reply(buf)) {
		i = get_int();
	} else {
		myuid = args->rk_userid;
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
rk_vilka_server(struct rk_vilka_args *rva)
{
	struct rk_dynamic_session_info_retval *pp;
	struct rk_dynamic_session_info *ppp;
	char buf[50];
	int antal, i;

	sprintf(buf, "83 %d %d %d\n", (rva->rva_flags & WHO_VISIBLE) != 0, 
	    (rva->rva_flags & WHO_INVISIBLE) != 0, rva->rva_secs);
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
rk_gettext_server(u_int32_t nr)
{
	char buf[50], *c;
	int i;

	sprintf(buf, "25 %d 0 2000000\n", nr);
	if (send_reply(buf)) {
		if ((i = get_int()))
			printf("Det sket sej: %d\n", i);
		get_eat('\n');
		return "";
	}
	c = get_string();
	get_accept('\n');
	return c;
}

static struct rk_misc_info *pmi;
//static struct rk_aux_item *pai;

struct rk_text_stat *
rk_textstat_server(u_int32_t nr)
{
	struct rk_text_stat *ts;
	char buf[30];
	int len, i;

	ts = calloc(sizeof(struct rk_text_stat), 1);
	sprintf(buf, "90 %d\n", nr);
	if (send_reply(buf)) {
		ts->rt_retval = get_int();
		get_eat('\n');
		return ts;
	}
	read_in_time(&ts->rt_time);
	ts->rt_author = get_int();
	ts->rt_no_of_lines = get_int();
	ts->rt_no_of_chars = get_int();
	ts->rt_no_of_marks = get_int();
	len = ts->rt_misc_info.rt_misc_info_len = get_int();
	if (len) {
		if (pmi)
			free(pmi);
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

	get_eat('\n'); /* XXX */
	return ts;
}


