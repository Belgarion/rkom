
#include <sys/types.h>
#include <sys/uio.h>

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "rkomsupport.h"
#include "backend.h"

extern int myuid;
extern FILE *sfd;

struct rk_time *
rk_time(void)
{
	static struct rk_time rkt;

	send_reply("35\n");
	read_in_time(&rkt);
	get_accept('\n');
	return &rkt;
}

static void
rk_alive_server_callback(int err, int arg)
{
	get_accept('\n');
}

void
rk_alive(void)
{
	send_callback("82\n", 0, rk_alive_server_callback);
}

int32_t
rk_whatido(char *args)
{
	int i;

	i = send_reply("4 %ldH%s\n", (long)strlen(args), args);
	get_accept('\n');
	return i;
}


struct rk_confinfo_retval *
rk_matchconf(char *name, u_int8_t flags)
{
	static struct rk_confinfo *rkc;
	static struct rk_confinfo_retval retval;
	int antal, i, p, k;

	if (rkc != NULL)
		free(rkc);

	k = (flags & MATCHCONF_CONF) == MATCHCONF_CONF;
	p = (flags & MATCHCONF_PERSON) == MATCHCONF_PERSON;
	send_reply("76 %ldH%s %d %d\n", (long)strlen(name), name, p, k);

	antal = get_int();
	rkc = calloc(sizeof(struct rk_confinfo), antal);
	if (antal == 0) {
		get_eat('\n');
	} else {
		retval.rcr_ci.rcr_ci_val = rkc;
		retval.rcr_ci.rcr_ci_len = antal;
		get_accept('{');
		for (i = 0; i < antal; i++) {
			rkc[i].rc_name = get_string();
			rkc[i].rc_type = get_bitfield();
			rkc[i].rc_type <<= 4; /* Careful: Extended-Conf-Type */
			rkc[i].rc_conf_no = get_int();
		}
		get_accept('}');
		get_accept('\n');
	}
	return &retval;
}

int32_t
rk_login(u_int32_t uid, char *pwd)
{
	int i;

	if (send_reply("62 %d %ldH%s 0\n", uid, (long)strlen(pwd), pwd)) {
		i = get_int();
	} else {
		myuid = uid;
		i = 0;
	}
	get_eat('\n');
	return i;
}

struct rk_unreadconfval *
rk_unreadconf(u_int32_t uid)
{
	static struct rk_unreadconfval rku;
	static u_int32_t *arr;
	int i, nconfs;

	if (arr != NULL)
		free(arr);
	arr = NULL;
	if ((i = send_reply("52 %d\n", uid))) {
		komerr = get_int();
		get_eat('\n');
		return NULL;
	}
	nconfs = get_int();
	rku.ru_confs.ru_confs_len = nconfs;
	if (nconfs == 0) {
		get_eat('\n');
	} else {
		arr = calloc(sizeof(u_int32_t), nconfs);
		rku.ru_confs.ru_confs_val = arr;
		get_accept('{');
		for (i = 0; i < nconfs; i++)
			arr[i] = get_int();
		get_accept('}');
		get_accept('\n');
	}
	return &rku;
}

struct rk_uconference *
rk_uconfinfo(u_int32_t mid) 
{
	static struct rk_uconference rku;

	if (send_reply("78 %d\n", mid)) {
		komerr = get_int();
		get_eat('\n');
		return NULL;
	}
	rku.ru_name = get_string();
	rku.ru_type = get_int();
	rku.ru_highest_local_no = get_int();
	rku.ru_nice = get_int();
	get_accept('\n');
	return &rku;
}

struct rk_dynamic_session_info_retval *
rk_vilka(u_int32_t secs, u_int32_t flags)
{
	static struct rk_dynamic_session_info_retval rkd;
	static struct rk_dynamic_session_info *ppp;
	int antal, i;

	if (ppp != NULL)
		free(ppp);
	send_reply("83 %d %d %d\n", (flags & WHO_VISIBLE) != 0, 
	    (flags & WHO_INVISIBLE) != 0, secs);

	antal = get_int();
	rkd.rdv_rds.rdv_rds_val =
	    calloc(sizeof(struct rk_dynamic_session_info), antal);
	rkd.rdv_rds.rdv_rds_len = antal;

	if (antal == 0) {
		get_eat('\n');
		return &rkd;
	}
	ppp = rkd.rdv_rds.rdv_rds_val;
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
	return &rkd;
}

char *
rk_client_name(u_int32_t vers)
{
	static char *ret;

	if (ret != NULL)
		free(ret);
	if (send_reply("70 %d\n", vers)) {
		get_eat('\n');
		return NULL;
	}
	ret = get_string();
	get_eat('\n');
	if (*ret == 0)
		ret = NULL;
	return ret;
}

char *
rk_client_version(u_int32_t vers)
{
	static char *ret;

	if (ret != NULL)
		free(ret);
	if (send_reply("71 %d\n", vers)) {
		get_eat('\n');
		return NULL;
	}
	ret = get_string();
	get_eat('\n');
	if (*ret == 0)
		ret = NULL;
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
rk_gettext(u_int32_t nr)
{
	struct text_stat_store *tss;
	char *c;
	int i;

	if ((tss = findtxt(nr)) && (tss->text))
		return strdup(tss->text);

	if (send_reply("25 %d 0 2000000\n", nr)) {
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
	tss->text = c;
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


void
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
rk_textstat(u_int32_t nr)
{
	struct text_stat_store *tss;
	struct rk_text_stat *ts;
	char buf[30];

	if ((tss = findtxt(nr)) && (tss->rts))
		return tss->rts;
	if (send_reply("90 %d\n", nr)) {
		komerr = get_int();
		get_eat('\n');
		return NULL;
	}
	ts = calloc(sizeof(struct rk_text_stat), 1);
	readin_textstat(ts);
	if (tss == 0) {
		tss = malloc(sizeof(*tss));
		tss->nummer = nr;
		tss->rts = NULL;
		tss->text = NULL;
		tss->next = pole;
		pole = tss;
	}
	if (tss->text == 0) {
		sprintf(buf, "25 %d 0 2000000\n", nr);
		send_callback(buf, nr, gettext_callback);
        }
	tss->rts = ts;
	return ts;

}

static void
reread_text_stat_bg_callback(int err, int arg)
{
	struct text_stat_store *tss;

	tss = findtxt(arg);
	if (err) {
		/* Text probably erased */
		get_eat('\n');
		free(tss->rts);
		tss->rts = 0;
		return;
	}
	tss->rts = calloc(sizeof(struct rk_text_stat), 1);
	readin_textstat(tss->rts);
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
	if (tss->rts) {
		ts = tss->rts;
		if (ts->rt_misc_info.rt_misc_info_len)
			free(ts->rt_misc_info.rt_misc_info_val);
		if (ts->rt_aux_item.rt_aux_item_len)
			free(ts->rt_aux_item.rt_aux_item_val);
		/* XXX loosing rai_data */
		free(tss->rts);
		tss->rts = NULL;
	}
	sprintf(buf, "90 %d\n", text);
	send_callback(buf, text, reread_text_stat_bg_callback);
}

/*
 * Put a text into a conference.
 */
u_int32_t
rk_create_text(struct rk_text_info *rti)
{
	struct rk_misc_info *mi;
	struct rk_aux_item_input *raii;
	u_int32_t textnr;
	int i, nmi, nraii;

	send_reply("86 %ldH", (long)strlen(rti->rti_text));
	fputs(rti->rti_text, sfd);
	nmi = rti->rti_misc.rti_misc_len;
	mi = rti->rti_misc.rti_misc_val;

	send_reply(" %d { ", nmi);
	for (i = 0; i < nmi; i++)
		send_reply("%d %d ", mi[i].rmi_type, mi[i].rmi_numeric);
	nraii = rti->rti_input.rti_input_len;
	raii = rti->rti_input.rti_input_val;
	send_reply("} %d { ", nraii);
	for (i = 0; i < nraii; i++) {
		send_reply("%d 00000000 %d ", raii[i].raii_tag,
		    raii[i].inherit_limit);
		send_reply("%ldH%s ", (long)strlen(raii[i].raii_data),
		    raii[i].raii_data);
	}

	if (send_reply("}\n")) {
		komerr = get_int();
		get_eat('\n');
		return 0;
	}
	textnr = get_int();
	get_accept('\n');

	return textnr;
}

/* Get the marked texts. */
struct rk_mark_retval *
rk_getmarks(void)
{
	static struct rk_mark_retval rkm;
	static struct rk_marks *rm;
	int cnt, i;

	if (send_reply("23\n")) {
		komerr = get_int();
		get_eat('\n');
		return NULL;
	}
	cnt = get_int();
	if (cnt) {
		if (rm != NULL)
			free(rm);
		rm = calloc(cnt, sizeof(*rm));
		rkm.rmr_marks.rmr_marks_len = cnt;
		rkm.rmr_marks.rmr_marks_val = rm;
		get_accept('{');
		for (i = 0; i < cnt; i++) {
			rm[i].rm_text = get_int();
			rm[i].rm_type = get_int();
		}
		get_accept('}');
	} else
		get_accept('*');
	get_accept('\n');
	return &rkm;
}

/* Mark a text */
int32_t
rk_setmark(u_int32_t text, u_int8_t type)
{
	int retval = 0;

	if (send_reply("72 %d %d\n", text, type))
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
rk_unmark(u_int32_t text)
{
	int retval = 0;

	if (send_reply("73 %d\n", text))
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

int32_t
rk_send_msg(u_int32_t dest, char *string)
{
	int i = 0;

	if (send_reply("53 %d %ldH%s\n", dest, (long)strlen(string), string))
		i = get_int();
	get_eat('\n');
	return i;
}

int32_t 
rk_setpass(u_int32_t uid, char *oldpass, char *newpass)
{
	int i;

	if (send_reply("8 %d %ldH%s %ldH%s\n", uid, (long)strlen(oldpass),
	    oldpass, (long)strlen(newpass), newpass)) {
		i = get_int();
		get_eat('\n');
		return i;
	}
	get_accept('\n');
	return 0;
}

int32_t
rk_change_name(u_int32_t uid, char *newname)
{
	int i;

	if (send_reply("3 %d %ldH%s\n", uid, (long)strlen(newname), newname)) {
		i = get_int();
		get_eat('\n');
		return i;
	}
	get_accept('\n');
	return 0;
}

int32_t
rk_set_presentation(u_int32_t conf, struct rk_text_info *rti)
{
	struct rk_misc_info *rkm;
	struct rk_conference *c;
	int i, presconf, oldn, nrkm;
	u_int32_t textno;
	void *old;

	/* Get conference info (pers/conf) */
	if ((c = rk_confinfo(conf)) == NULL)
		return 9;

	/* Get presentation conference number */
	send_reply("94\n");
	get_int();
	presconf = get_int();
	if (c->rc_type & RK_CONF_TYPE_LETTERBOX)
		presconf = get_int();
	else
		get_int();
	get_int();get_int();get_int();
	get_eat('\n'); /* XXX */

	/* Add receiving conference */
	old = rti->rti_misc.rti_misc_val;
	oldn = rti->rti_misc.rti_misc_len;

	nrkm = rti->rti_misc.rti_misc_len + 1;
	rkm = alloca(sizeof(struct rk_misc_info) * nrkm);
	if (rti->rti_misc.rti_misc_len)
		memcpy(rkm, rti->rti_misc.rti_misc_val,
		    sizeof(struct rk_misc_info) * rti->rti_misc.rti_misc_len);
	rkm[nrkm-1].rmi_type = recpt;
	rkm[nrkm-1].rmi_numeric = presconf;
	rti->rti_misc.rti_misc_val = rkm;
	rti->rti_misc.rti_misc_len = nrkm;

	/* create text */
	if ((textno = rk_create_text(rti)) == 0)
		return komerr;

	rti->rti_misc.rti_misc_len = oldn;
	rti->rti_misc.rti_misc_val = old;
	/* Set text as presentation */
	if (send_reply("16 %d %d\n", conf, textno)) {
		i = get_int();
		get_eat('\n');
		return i;
	}
	get_accept('\n');
	return 0;
}

int32_t
rk_add_rcpt(u_int32_t text, u_int32_t conf, u_int32_t type)
{
	int i;

	if (send_reply("30 %d %d %d\n", text, conf, type)) {
		i = get_int();
		get_eat('\n');
		return i;
	}
	get_accept('\n');
	return 0;
}

int32_t
rk_sub_rcpt(u_int32_t text, u_int32_t conf)
{
	int i;

	if (send_reply("31 %d %d\n", text, conf)) {
		i = get_int();
		get_eat('\n');
		return i;
	}
	get_accept('\n');
	return 0;
}

int32_t
rk_delete_text(u_int32_t text)
{
	int i;

	if (send_reply("29 %d\n", text)) {
		i = get_int();
		get_eat('\n');
		return i;
	}
	get_accept('\n');
	return 0;
}

int32_t
rk_set_motd(u_int32_t conf, struct rk_text_info *rti)
{
	int i, motdconf, oldn;
	u_int32_t textnr;
	void *old;

	if (*rti->rti_text == 0) { /* Remove motd */
		if (send_reply("17 %d 0\n", conf)) {
			i = get_int();
			get_eat('\n');
			return i;
		}
		get_accept('\n');
		return 0;
	}
	/* Get presentation conference number */
	send_reply("94\n");
	get_int();
	get_int();get_int();
	motdconf = get_int();
	get_int();get_int();
	get_eat('\n'); /* XXX */

	/* Add receiving conference */
	old = rti->rti_misc.rti_misc_val;
	oldn = rti->rti_misc.rti_misc_len;
	rti->rti_misc.rti_misc_val = alloca(sizeof(struct rk_misc_info));
	rti->rti_misc.rti_misc_val->rmi_type = recpt;
	rti->rti_misc.rti_misc_val->rmi_numeric = motdconf;
	rti->rti_misc.rti_misc_len = 1;

	/* create text */
	textnr = rk_create_text(rti);
	if (textnr == 0)
		return komerr;

	rti->rti_misc.rti_misc_len = oldn;
	rti->rti_misc.rti_misc_val = old;
	/* Set text as presentation */
	if (send_reply("17 %d %d\n", conf, textnr)) {
		i = get_int();
		get_eat('\n');
		return i;
	}
	get_accept('\n');
	return 0;
}

int32_t
rk_add_text_info(u_int32_t textno, struct rk_aux_item_input *raii)
{
	int ret;

	send_reply("92 %ld 0 { }", (long)textno);
	send_reply(" 1 { %d 00000000 %d %ldH", raii->raii_tag,
		raii->inherit_limit, (long)strlen(raii->raii_data));
	fputs(raii->raii_data, sfd);
	send_reply(" }\n");

	ret = get_int();
	get_eat('\n');

	return ret;
}
