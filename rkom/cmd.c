
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pwd.h>
#include <histedit.h>

#include "rkom_proto.h"
#include "rkom.h"
#include "next.h"
#include "list.h"
#include "write.h"
#include "set.h"

void cmd_logout(char *);
void cmd_login(char *);
void cmd_tiden(char *);
void cmd_vilka(char *);
void cmd_sluta(char *);
void cmd_send(char *);
void cmd_say(char *);
void cmd_where(char *);
void cmd_goto(char *);
void cmd_leave(char *);

int myuid = 0, curconf = 0;

static char *dindx[] = {"sön", "mån", "tis", "ons", "tors", "fre", "lör"};
static char *mindx[] = {"januari", "februari", "mars", "april", "maj", "juni",
	"juli", "augusti", "september", "oktober", "november", "december"};

static char *tindx[] = {"tolv", "ett", "två", "tre", "fyra", "fem", "sex",
	"sju", "åtta", "nio", "tio", "elva" };

static char *dyidx[] = {"natten", "morgonen", "eftermiddagen", "kvällen" };

void
cmd_tiden(char *str)
{
	struct rk_time *tm;

	tm = rk_time();

	rprintf("Klockan är ");
	if (tm->rt_minutes > 30)
		rprintf("%d minut%s i %s",  60 - tm->rt_minutes,
		    tm->rt_minutes == 59 ? "" : "er",
		    tindx[((tm->rt_hours + 1)%12)]);
	else if (tm->rt_minutes == 30)
		rprintf("halv %s", tindx[((tm->rt_hours+1)%12)]);
	else if (tm->rt_minutes == 0)
		rprintf("prick %s", tindx[(tm->rt_hours%12)]);
	else
		rprintf("%d minut%s över %s", tm->rt_minutes,
		    tm->rt_minutes == 1 ? "" : "er", tindx[(tm->rt_hours%12)]);
	rprintf(" på %s", dyidx[tm->rt_hours/6]);
	rprintf(",\n%sdagen den %d %s %d", dindx[tm->rt_day_of_week], tm->rt_day,
	    mindx[tm->rt_month], tm->rt_year + 1900);
	if (tm->rt_is_dst)
		rprintf(" (sommartid)");
	rprintf(" (enligt servern)\n");
}

static char *
nxtcmd(char **str)
{
	char *h, *g;

	g = *str;
	if (g == 0)
		return 0;
	while (isspace(*g))
		g++;
	if (*g == 0)
		return 0;
	h = g;
	while (isalpha(*h) || isdigit(*h))
		h++;
	if (*h == 0) {
		*str = 0;
		return g;
	}
	*h++ = 0;
	*str = h;
	return g;
}

void
cmd_vilka(char *str)
{
	struct rk_dynamic_session_info_retval *ppp;
	struct rk_dynamic_session_info *pp;
	int i, antal, invisible, visible, clients, type, idle;
	char *s;

	invisible = visible = clients = idle = 0;
	if (str == 0)
		visible++;
	else
		while ((s = nxtcmd(&str))) {
			if (bcmp(s, "osynliga", strlen(s)) == 0)
				invisible++;
			if (bcmp(s, "synliga", strlen(s)) == 0)
				visible++;
			if (bcmp(s, "klienter", strlen(s)) == 0)
				clients++;
			if (atoi(s))
				idle = atoi(s);
		}
	type = 0;
	if (visible)
		type |= WHO_VISIBLE;
	if (invisible)
		type |= WHO_INVISIBLE;
	if (type == 0)
		type = WHO_VISIBLE;
	ppp = rk_vilka(idle, type);

	antal = ppp->rdv_rds.rdv_rds_len;
	pp = ppp->rdv_rds.rdv_rds_val;
	if (antal == 0) {
		rprintf("Det är inga påloggade alls.\n");
		free(ppp);
		return;
	}

	rprintf("Det är %d person%s påloggade.\n",
	    antal, antal == 1 ? "" : "er");
	rprintf("-------------------------------------------------------\n");

	for (i = 0; i < antal; i++) {
		struct rk_conference *c1, *c2;
		struct rk_person *p;
		char *name, *conf, *var;

		if (pp[i].rds_person == 0)
			continue;
		c1 = rk_confinfo(pp[i].rds_person);
		name = c1->rc_name;
		if (pp[i].rds_conf) {
			c2 = rk_confinfo(pp[i].rds_conf);
			conf = c2->rc_name;
		} else
			conf = "Inte närvarande någonstans";
		p = rk_persinfo(pp[i].rds_person);
		var = p->rp_username;
		if (strlen(name) > 33)
			rprintf("%5d %s\n%40s%s\n",
			    pp[i].rds_session, name, "", conf);
		else
			rprintf("%5d %-33s %-40s\n",
			    pp[i].rds_session, name, conf);
		if (strlen(var) > 36)
			rprintf("   %s\n%40s%s\n", var, "", pp[i].rds_doing);
		else
			rprintf("   %-37s%s\n", var, pp[i].rds_doing);
		if (clients) {
			char *nn, *vv;

			nn = rk_client_name(pp[i].rds_session);
			vv = rk_client_version(pp[i].rds_session);
			rprintf("   %-37s", (strlen(nn) == 0 ? "(Okänd)" : nn));
			rprintf("%s\n", (strlen(vv) == 0 ? "(Okänt)" : vv));
			free(nn);
			free(vv);
		}
		rprintf("\n");
		free(c1);
		if (pp[i].rds_conf)
			free(c2);
		free(p);
	}
	rprintf("-------------------------------------------------------\n");
	free(ppp);
}

void
cmd_login(char *str)
{
	struct rk_confinfo_retval *retval;
	struct rk_unreadconfval *conf;
	int nconf, userid;
	char *passwd;

	if (str == 0) {
		rprintf("Du måste ange vem du vill logga in som.\n");
		return;
	}
	retval = match_complain(str, MATCHCONF_PERSON);
	if (retval == NULL)
		return;

	rprintf("%s\n", retval->rcr_ci.rcr_ci_val[0].rc_name);
	userid = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	passwd = getpass(swascii ? "L|senord: " : "Lösenord: ");
	if (rk_login(userid, passwd)) {
		rprintf("Felaktigt lösenord.\n\n");
		free(retval);
		return;
	}
	myuid = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	free(retval);

	readvars();
	/*
	 * Set some informative text.
	 */
	if (rk_whatido("Kör raggeklienten (nu med login!)"))
		rprintf("Set what-i-am-doing sket sej\n");

	/* Show where we have unread texts. */
	if (iseql("print-number-of-unread-on-entrance", "1"))
		list_news(0);

	conf = rk_unreadconf(myuid);
	nconf = conf->ru_confs.ru_confs_len;
	if (nconf)
		prompt = PROMPT_NEXT_CONF;
	free(conf);
}

void
cmd_logout(char *str)
{
	rkom_logout();
}

void
cmd_sluta(char *str)
{
	rprintf("Sluta\n");
	if (is_writing) {
		rprintf("Du håller på att skriva en text. ");
		rprintf("Lägg in eller glöm den först.\n");
		return;
	}
	rprintf("Nu avslutar du rkom.\n");
	exit(0);
}

void 
cmd_send(char *str)
{
	char *buf;

	rprintf("Sänd (alarmmeddelande till alla)\n");

	buf = getstr("Meddelande: ");

	if (strlen(buf) == 0)
		rprintf("Nähej.");
	else
		rk_send_msg(0, buf);
	free(buf);
}

void 
cmd_say(char *str)
{
	struct rk_confinfo_retval *retval;
	char *buf;

	if (str == 0) {
		rprintf("Du måste ange vem du vill skicka meddelande till.\n");
		return;
	}
	retval = match_complain(str, MATCHCONF_PERSON|MATCHCONF_CONF);
	if (retval == 0)
		return;

	rprintf("Sänd meddelande till %s\n",
	    retval->rcr_ci.rcr_ci_val[0].rc_name);

	buf = getstr("Meddelande: ");
	if (strlen(buf) == 0)
		rprintf("Nähej.");
	else {
		rk_send_msg(retval->rcr_ci.rcr_ci_val[0].rc_conf_no, buf);
		rprintf("\nMeddelandet sänt till %s.\n", 
		    retval->rcr_ci.rcr_ci_val[0].rc_name);
	}
	free(buf);
	free(retval);
}

void
cmd_where(char *str)
{
	struct rk_conference *conf;

	if (myuid == 0)
		rprintf("Du är inte ens inloggad.\n");
	else if (curconf == 0)
		rprintf("Du är inte närvarande någonstans.\n");
	else {
		conf = rk_confinfo(curconf);
		rprintf("Du är i möte %s.\n", conf->rc_name);
		free(conf);
	}
}

void
cmd_goto(char *str)
{
	struct rk_conference *rkc;
	struct rk_membership *m;
	struct rk_confinfo_retval *retval;
	int conf, ret;
	char *ch, *name;

	if (myuid == 0) {
		rprintf("Logga in först.\n");
		return;
	}
	if (str == 0) {
		rprintf("Du måste ge ett möte som argument.\n");
		return;
	}
	retval = match_complain(str, MATCHCONF_CONF|MATCHCONF_PERSON);
	if (retval == NULL)
		return;
	conf = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	name = alloca(strlen(retval->rcr_ci.rcr_ci_val[0].rc_name) + 2);
	strcpy(name, retval->rcr_ci.rcr_ci_val[0].rc_name);
	free(retval);
	ret = rk_change_conference(conf);
	if (ret == 0) {
		curconf = conf;
		rprintf("Du gick nu till möte %s.\n", name);
		next_resetchain();
		return;
	}
	/* XXX Check why change_conference failed; status of conf etc */
	if (ret == 13) { /* XXX should be define */
		rprintf("Du är inte medlem i %s.\n", name);
		do {
			ch = getstr("Vill du bli medlem? (ja, nej) - ");
		} while (strcasecmp("ja", ch) && strcasecmp("nej", ch));
		if (strcasecmp("nej", ch) == 0) {
			rprintf("Nehepp.\n");
			next_resetchain();
			return;
		}
	} else if (ret != 0) {
		rprintf("%s\n", error(ret));
		return;
	}
	if ((ret = rk_add_member(conf, myuid, 100, 3, 0))) {
		if (ret == 11)
			rprintf("Mötet är slutet, kontakta administratören "
			    "för medlemsskap.\n");
		else
			rprintf("%s\n", error(ret));
		return;
	}
	curconf = conf;
	rprintf("Du är nu medlem i %s.\n", name);
	rkc = rk_confinfo(conf);
	m = rk_membership(myuid, conf);
	ret = rkc->rc_first_local_no + rkc->rc_no_of_texts - 1 -
	    m->rm_last_text_read;
	if (ret) {
		rprintf("\nDu har %d oläst%s inlägg.\n", ret, ret>1?"a":"");
	} else {
		rprintf("\nDu har inga olästa inlägg.\n");
	}
	next_resetchain();
}

void
cmd_only(char *str)
{
	struct rk_conference *conf;
	int only, high;

	only = atoi(str);
	conf = rk_confinfo(curconf);
	high = conf->rc_first_local_no + conf->rc_no_of_texts - 1;
	rk_set_last_read(curconf, high - only);
	next_resetchain();
	next_prompt();
}

void
cmd_leave(char *str)
{
	struct rk_confinfo_retval *retval;
	int ret;

	retval = match_complain(str, MATCHCONF_CONF);
	if (retval == NULL)
		return;
	ret = rk_sub_member(retval->rcr_ci.rcr_ci_val[0].rc_conf_no, myuid);
	if (ret)
		rprintf("Det sket sej: %s\n", error(ret));
	free(retval);
}

void
cmd_password()
{
	char *opass, *npass1, *npass2;
	int rv;

	rprintf("Ändra lösenord\n");
	opass = strdup(getpass("Ange gamla lösenordet: "));
	npass1 = strdup(getpass("Ange nya lösenordet: "));
	npass2 = strdup(getpass("Ange nya lösenordet igen: "));

	if (strcmp(npass1, npass2)) {
		rprintf("Du skrev olika nya lösenord, försök igen.\n");
	} else {
		rv = rk_setpass(myuid, opass, npass1);
		if (rv)
			rprintf("Det sket sej: %s\n", error(rv));
	}
	free(opass);
	free(npass1);
	free(npass2);
}

static void
persstat(int uid)
{
	struct rk_person *p;

	p = rk_persinfo(uid);
	rprintf("Namn:                 %s\n", vem(uid));
	rprintf("Person nummer:        %-5d\n", uid);
	rprintf("Inloggad från:        %s\n", p->rp_username);
	rprintf("Senast inloggad:      %s\n",
	    get_date_string(&p->rp_last_login));
	rprintf("Total närvarotid:     %dd %d:%d.%d\n",
	    p->rp_total_time_present/(60*60*24),
	    p->rp_total_time_present/(60*60) % 24,
	    p->rp_total_time_present/60 % 60,
	    p->rp_total_time_present % 60);
	rprintf("Skrivna texter:       %d\n", p->rp_no_of_created_texts);
	rprintf("Skrivna rader:        %d\n", p->rp_created_lines);
	rprintf("Lästa texter:         %d\n", p->rp_read_texts);
	rprintf("Markerade texter:     %d\n", p->rp_no_of_marks);

	free(p);
}

static void
confstat(int mid)
{
	struct rk_conference *rcp;

	rcp = rk_confinfo(mid);
	rprintf("Namn:                  %s\n", vem(mid));
	rprintf("Antal texter:          %d\n", rcp->rc_no_of_texts);
	rprintf("Skapat:                %s\n",
		get_date_string(&rcp->rc_creation_time));
	rprintf("Skapare:               %s\n",
		vem(rcp->rc_creator));
	rprintf("Senaste text:          %s\n",
		get_date_string(&rcp->rc_last_written));
	rprintf("Antal medlemmar:       %d\n",
		rcp->rc_no_of_members);
	rprintf("Livslängd:             %d dagar\n",
		rcp->rc_expire);
}

void
cmd_status(char *name)
{
	struct rk_confinfo_retval *retval;

	retval = match_complain(name, MATCHCONF_CONF|MATCHCONF_PERSON);
	if (retval == NULL)
		return;
	rprintf("Status (för) %s\n\n", retval->rcr_ci.rcr_ci_val[0].rc_name);
	if (retval->rcr_ci.rcr_ci_val[0].rc_type & RK_CONF_TYPE_LETTERBOX)
		persstat(retval->rcr_ci.rcr_ci_val[0].rc_conf_no);
	else
		confstat(retval->rcr_ci.rcr_ci_val[0].rc_conf_no);
	free(retval);
}

void
cmd_info_extra(int text)
{
	struct rk_text_stat *rts;
	struct rk_aux_item *rai;
	int i, nrai;

	printf("Tilläggsinformation (för text) %d.\n\n", text);

	rts = rk_textstat(text);
	if (rts->rt_retval) {
		printf("Kunde inte läsa status för inlägg %d: %s.\n",
		    text, error(rts->rt_retval));
		free(rts);
		return;
	}
	nrai = rts->rt_aux_item.rt_aux_item_len;
	rai = rts->rt_aux_item.rt_aux_item_val;
	if (nrai) {
		for (i = 0; i < nrai; i++) {
			printf("Nummer:\t\t%d\n", rai[i].rai_aux_no);
			printf("Typ:\t\t%d\n", rai[i].rai_tag);
			printf("Skapad av:\t%s\n", vem(rai[i].rai_creator));
			printf("Skapad:\t\t%s\n",
			    get_date_string(&rai[i].rai_created_at));
			printf("Flaggor:\t\n");
			printf("Arvsgräns:\t%d\n", rai[i].inherit_limit);
			printf("Innehåll:\t\"%s\".\n", rai[i].rai_data);
		}
	} else
		printf("Det finns ingen tilläggsinformation för denna text.\n");
	free(rts);
}

void
cmd_change_name()
{
	struct rk_confinfo_retval *retval;
	int rv;
	char *name;

	printf("Ändra namn\n\n");

	name = getstr("Vilket namn skall ändras? ");
	if (strlen(name) == 0) {
		printf("Nähej.\n");
		free(name);
		return;
	}
	retval = match_complain(name, MATCHCONF_CONF|MATCHCONF_PERSON);
	free(name);
	if (retval == NULL)
		return;
	printf("%s\n", retval->rcr_ci.rcr_ci_val[0].rc_name);
	name = getstr("Vad skall det ändras till? ");
	rv = rk_change_name(retval->rcr_ci.rcr_ci_val[0].rc_conf_no, name);
	if (rv)
		printf("Det gick inte: %s\n", error(rv));
	free(retval);
	free(name);
}

void
cmd_add_member()
{
	struct rk_confinfo_retval *retval;
	int rv, uid, mid;
	char *name, *user;

	printf("Addera medlem\n\n");

	name = getstr("Vem skall adderas? ");
	if (strlen(name) == 0) {
		printf("Nähej.\n"); 
		free(name);
		return;
	}
	retval = match_complain(name, MATCHCONF_PERSON);
	free(name);
	if (retval == NULL)
		return;
	printf("%s\n", retval->rcr_ci.rcr_ci_val[0].rc_name);
	user = strdup(retval->rcr_ci.rcr_ci_val[0].rc_name);
	uid = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	free(retval);
	name = getstr("Till vilket möte? ");
	if (strlen(name) == 0) {
		printf("Nähej.\n");
		free(name);
		free(user);
		return;
	}
	retval = match_complain(name, MATCHCONF_CONF);
	free(name);
	if (retval == NULL) {
		free(user);
		return;
	}
	mid = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	rv = rk_add_member(mid, uid, 100, 3, 0);
	if (rv)
		printf("Det gick inte: %s\n", error(rv));
	else
		printf("Person %s adderad till möte %s.\n",
		    user, retval->rcr_ci.rcr_ci_val[0].rc_name);
	free(retval);
	free(user);
}

void
cmd_sub_member()
{
	struct rk_confinfo_retval *retval;
	int rv, uid, mid;
	char *name;

	printf("Subtrahera medlem\n\n");

	name = getstr("Vem skall subtraheras? ");
	if (strlen(name) == 0) {
		printf("Nähej.\n"); 
		free(name);
		return;
	}
	retval = match_complain(name, MATCHCONF_PERSON);
	free(name);
	if (retval == NULL)
		return;
	printf("%s\n", retval->rcr_ci.rcr_ci_val[0].rc_name);
	uid = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	free(retval);
	name = getstr("Från vilket möte? ");
	if (strlen(name) == 0) {
		printf("Nähej.\n");
		free(name);
		return;
	}
	retval = match_complain(name, MATCHCONF_CONF);
	free(name);
	if (retval == NULL)
		return;
	mid = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	free(retval);
	rv = rk_sub_member(mid, uid);
	if (rv)
		printf("Det gick inte: %s\n", error(rv));
}

void    
cmd_add_rcpt()
{
	struct rk_confinfo_retval *retval;
	int rv, conf, text;
	char *name, buf[50];

	printf("Addera mottagare\n\n");

	name = getstr("Vilket möte skall adderas? ");
	if (strlen(name) == 0) {
		printf("Nähej.\n"); 
		free(name);
		return;
	}
	retval = match_complain(name, MATCHCONF_CONF);
	free(name);
	if (retval == NULL)
		return;
	printf("%s\n", retval->rcr_ci.rcr_ci_val[0].rc_name);
	conf = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	if (lasttext)
		sprintf(buf, "Till vilken text? (%d) ", lasttext);
	else
		sprintf(buf, "Till vilken text? ");
	name = getstr(buf);
	if ((strlen(name) == 0) && lasttext) {
		text = lasttext;
	} else if (strlen(name) == 0) {
		printf("Nähej.\n");
		free(name);
		free(retval);
		return;
	} else 
		text = atoi(name);
	free(name);
	if (text == 0) {
		printf("Det var ett dåligt textnummer.\n");
		free(retval);
		return;
	}
	rv = rk_add_rcpt(text, conf, recpt);
	if (rv)
		printf("Det gick inte: %s\n", error(rv));
	else
		printf("Text %d adderad till möte %s.\n", text,
		    retval->rcr_ci.rcr_ci_val[0].rc_name);
	free(retval);
}

void 
cmd_sub_rcpt()
{
	struct rk_confinfo_retval *retval;
	int rv, conf, text;
	char *name, buf[50];

	printf("Subtrahera mottagare \n\n");

	name = getstr("Vilket möte skall subtraheras? ");
	if (strlen(name) == 0) {
		printf("Nähej.\n"); 
		free(name);
		return;
	}
	retval = match_complain(name, MATCHCONF_CONF);
	free(name);
	if (retval == NULL)
		return;
	printf("%s\n", retval->rcr_ci.rcr_ci_val[0].rc_name);
	conf = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	if (lasttext)
		sprintf(buf, "Från vilken text? (%d) ", lasttext);
	else
		sprintf(buf, "Från vilken text? ");
	name = getstr(buf);
	if ((strlen(name) == 0) && lasttext) {
		text = lasttext;
	} else if (strlen(name) == 0) {
		printf("Nähej.\n");
		free(name);
		free(retval);
		return;
	} else
		text = atoi(name);
	free(name);
	if (text == 0) {
		printf("Det var ett dåligt textnummer.\n");
		free(retval);
		return;
	}
	rv = rk_sub_rcpt(text, conf);
	if (rv)
		printf("Det gick inte: %s\n", error(rv));
	else
		printf("Text %d subtraherad från möte %s.\n", text,
		    retval->rcr_ci.rcr_ci_val[0].rc_name);
	free(retval);
}

void
cmd_delete(int text)
{
	int rv;

	rv = rk_delete_text(text);
	if (rv)
		printf("Det gick inte: %s\n", error(rv));
	else
		printf("Text %d nu raderad.\n", text);
}
