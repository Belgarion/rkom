
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pwd.h>

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

static char *dindx[] = {"s�n", "m�n", "tis", "ons", "tors", "fre", "l�r"};
static char *mindx[] = {"januari", "februari", "mars", "april", "maj", "juni",
	"juli", "augusti", "september", "oktober", "november", "december"};

static char *tindx[] = {"tolv", "ett", "tv�", "tre", "fyra", "fem", "sex",
	"sju", "�tta", "nio", "tio", "elva" };

static char *dyidx[] = {"natten", "morgonen", "eftermiddagen", "kv�llen" };

void
cmd_tiden(char *str)
{
	struct rk_time *tm;

	tm = rk_time(0);

	printf("Klockan �r ");
	if (tm->rt_minutes > 30)
		printf("%d minut%s i %s",  60 - tm->rt_minutes,
		    tm->rt_minutes == 59 ? "" : "er",
		    tindx[((tm->rt_hours + 1)%12)]);
	else if (tm->rt_minutes == 30)
		printf("halv %s", tindx[((tm->rt_hours+1)%12)]);
	else if (tm->rt_minutes == 0)
		printf("prick %s", tindx[(tm->rt_hours%12)]);
	else
		printf("%d minut%s �ver %s", tm->rt_minutes,
		    tm->rt_minutes == 1 ? "" : "er", tindx[(tm->rt_hours%12)]);
	printf(" p� %s", dyidx[tm->rt_hours/6]);
	printf(",\n%sdagen den %d %s %d", dindx[tm->rt_day_of_week], tm->rt_day,
	    mindx[tm->rt_month], tm->rt_year + 1900);
	if (tm->rt_is_dst)
		printf(" (sommartid)");
	printf(" (enligt servern)\n");
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
		printf("Det �r inga p�loggade alls.\n");
		free(ppp);
		return;
	}

	printf("Det �r %d person%s p�loggade.\n",
	    antal, antal == 1 ? "" : "er");
	printf("-------------------------------------------------------\n");

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
			conf = "Inte n�rvarande n�gonstans";
		p = rk_persinfo(pp[i].rds_person);
		var = p->rp_username;
		if (strlen(name) > 33)
			printf("%5d %s\n%40s%s\n",
			    pp[i].rds_session, name, "", conf);
		else
			printf("%5d %-33s %-40s\n",
			    pp[i].rds_session, name, conf);
		if (strlen(var) > 36)
			printf("   %s\n%40s%s\n", var, "", pp[i].rds_doing);
		else
			printf("   %-37s%s\n", var, pp[i].rds_doing);
		if (clients) {
			char *nn, *vv;

			nn = rk_client_name(pp[i].rds_session);
			vv = rk_client_version(pp[i].rds_session);
			printf("   %-37s", (strlen(nn) == 0 ? "(Ok�nd)" : nn));
			printf("%s\n", (strlen(vv) == 0 ? "(Ok�nt)" : vv));
			free(nn);
			free(vv);
		}
		printf("\n");
		free(c1);
		if (pp[i].rds_conf)
			free(c2);
		free(p);
	}
	printf("-------------------------------------------------------\n");
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
		printf("Du m�ste ange vem du vill logga in som.\n");
		return;
	}
	retval = match_complain(str, MATCHCONF_PERSON);
	if (retval == NULL)
		return;

	printf("%s\n", retval->rcr_ci.rcr_ci_val[0].rc_name);
	userid = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	passwd = getpass("L�senord: ");
	if (rk_login(userid, passwd)) {
		printf("Felaktigt l�senord.\n\n");
		free(retval);
		return;
	}
	myuid = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	free(retval);

	readvars();
	/*
	 * Set some informative text.
	 */
	if (rk_whatido("K�r raggeklienten (nu med login!)"))
		printf("Set what-i-am-doing sket sej\n");

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
	printf("Sluta\n");
	if (is_writing) {
		printf("Du h�ller p� att skriva en text. ");
		printf("L�gg in eller gl�m den f�rst.\n");
		return;
	}
	printf("Nu avslutar du rkom.\n");
	exit(0);
}

void 
cmd_send(char *str)
{
	char *buf;

	printf("S�nd (alarmmeddelande till alla)\nMeddelande: ");
	fflush(stdout);

	buf = get_input_string(0, 0); /* XXX */

	if (strlen(buf) == 0)
		printf("N�hej.");
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
		printf("Du m�ste ange vem du vill skicka meddelande till.\n");
		return;
	}
	retval = match_complain(str, MATCHCONF_PERSON|MATCHCONF_CONF);
	if (retval == 0)
		return;

	printf("S�nd meddelande till %s\nMeddelande: ",
	    retval->rcr_ci.rcr_ci_val[0].rc_name);
	fflush(stdout);
	buf = get_input_string(0, 0); /* XXX */
	if (strlen(buf) == 0)
		printf("N�hej.");
	else {
		rk_send_msg(retval->rcr_ci.rcr_ci_val[0].rc_conf_no, buf);
		printf("\nMeddelandet s�nt till %s.\n", 
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
		printf("Du �r inte ens inloggad.\n");
	else if (curconf == 0)
		printf("Du �r inte n�rvarande n�gonstans.\n");
	else {
		conf = rk_confinfo(curconf);
		printf("Du �r i m�te %s.\n", conf->rc_name);
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
		printf("Logga in f�rst.\n");
		return;
	}
	if (str == 0) {
		printf("Du m�ste ge ett m�te som argument.\n");
		return;
	}
	retval = match_complain(str, MATCHCONF_CONF);
	if (retval == NULL)
		return;
	conf = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	name = alloca(strlen(retval->rcr_ci.rcr_ci_val[0].rc_name) + 2);
	strcpy(name, retval->rcr_ci.rcr_ci_val[0].rc_name);
	free(retval);
	ret = rk_change_conference(conf);
	if (ret == 0) {
		curconf = conf;
		printf("Du gick nu till m�te %s.\n", name);
		next_resetchain();
		return;
	}
	/* XXX Check why change_conference failed; status of conf etc */
	if (ret == 13) { /* XXX should be define */
		printf("Du �r inte medlem i %s.\n", name);
		do {
			printf("Vill du bli medlem? (ja, nej) - ");
			fflush(stdout);
			ch = get_input_string(0, 0); /* XXX */
		} while (strcasecmp("ja", ch) && strcasecmp("nej", ch));
		if (strcasecmp("nej", ch) == 0) {
			printf("Nehepp.\n");
			next_resetchain();
			return;
		}
	} else if (ret != 0) {
		printf("%s\n", error(ret));
		return;
	}
	if ((ret = rk_add_member(conf, myuid, 100, 3, 0))) {
		printf("%s\n", error(ret));
		return;
	}
	curconf = conf;
	printf("Du �r nu medlem i %s.\n", name);
	rkc = rk_confinfo(conf);
	m = rk_membership(myuid, conf);
	ret = rkc->rc_first_local_no + rkc->rc_no_of_texts - 1 -
	    m->rm_last_text_read;
	if (ret) {
		printf("\nDu har %d ol�sta inl�gg.\n", ret);
	} else {
		printf("\nDu har inga ol�sta inl�gg.\n");
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

	if (myuid == 0) {
		printf("Logga in f�rst.\n");
		return;
	}
	if (str == 0) {
		printf("Du m�ste ge ett m�te som argument.\n");
		return;
	}
	retval = match_complain(str, MATCHCONF_CONF);
	if (retval == NULL)
		return;
	ret = rk_sub_member(retval->rcr_ci.rcr_ci_val[0].rc_conf_no, myuid);
	if (ret)
		printf("Det sket sej: %s\n", error(ret));
	free(retval);
}

void
cmd_password()
{
	char *opass, *npass1, *npass2;
	int rv;

	printf("�ndra l�senord\n");
	opass = strdup(getpass("Ange gamla l�senordet: "));
	npass1 = strdup(getpass("Ange nya l�senordet: "));
	npass2 = strdup(getpass("Ange nya l�senordet igen: "));

	if (strcmp(npass1, npass2)) {
		printf("Du skrev olika nya l�senord, f�rs�k igen.\n");
	} else {
		rv = rk_setpass(myuid, opass, npass1);
		if (rv)
			printf("Det sket sej: %s\n", error(rv));
	}
	free(opass);
	free(npass1);
	free(npass2);
}
