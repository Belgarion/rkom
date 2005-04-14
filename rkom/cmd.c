/*	$Id: cmd.c,v 1.78 2005/04/14 15:51:10 ragge Exp $	*/

#if defined(SOLARIS)
#undef _XPG4_2
#endif

#ifdef LINUX
#include <string.h>
#endif
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>

#include "rkomsupport.h"
#include "rkom.h"
#include "backend.h"
#include "next.h"
#include "list.h"
#include "write.h"
#include "set.h"
#include "rhistedit.h"
#include "rtype.h"

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

	if(iseql("short-time-format","1")) {
		rprintf("Det är %sdag %s (enligt servern).\n",
			dindx[tm->rt_day_of_week],
			get_date_string(tm));
		return;
	}

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
	while (isspace((int)*g))
		g++;
	if (*g == 0)
		return 0;
	h = g;
	while (isalpha((int)*h) || isdigit((int)*h))
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
	struct rk_dynamic_session_info *pp;
	int i, antal, invisible, visible, clients, type, idle;
	char *s;

	invisible = visible = clients = 0;
	idle = atoi(getval("idle-hide-in-who-list"));
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
	idle *= 60; /* In minutes */
	type = 0;
	if (visible)
		type |= WHO_VISIBLE;
	if (invisible)
		type |= WHO_INVISIBLE;
	if (type == 0)
		type = WHO_VISIBLE;
	pp = rk_vilka(idle, type);

	for (antal = 0; pp[antal].rds_session; antal++)
		;
	if (antal == 0)
		return rprintf("Det är inga påloggade alls.\n");

	rprintf("Det är %d person%s påloggade.\n",
	    antal, antal == 1 ? "" : "er");
	rprintf("-------------------------------------------------------\n");

	for (i = 0; i < antal; i++) {
		struct rk_conference *c;
		struct rk_person *p;
		char *name, *conf, *var;

		if (pp[i].rds_person == 0)
			continue;
		if ((c = rk_confinfo(pp[i].rds_person)) == NULL)
			name = "Okänd";
		else
			name = c->rc_name;
		if (pp[i].rds_conf) {
			if ((c = rk_confinfo(pp[i].rds_conf)) == NULL)
				conf = "Okänd";
			else
				conf = c->rc_name;
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
			rprintf("   %-37s", (nn == 0 ? "(Okänd)" : nn));
			rprintf("%s\n", (vv == 0 ? "(Okänt)" : vv));
		}
		rprintf("\n");
	}
	rprintf("-------------------------------------------------------\n");
}

void
cmd_login(char *str)
{
	struct rk_confinfo *rv;
	struct rk_conference *rc;
	int userid;
	u_int32_t *c;
	char *passwd;

	if (str == 0)
		return rprintf("Du måste ange vem du vill logga in som.\n");

	if ((rv = match_complain(str, MATCHCONF_PERSON)) == NULL)
		return;

	rprintf("%s\n", rv[0].rc_name);
	userid = rv[0].rc_conf_no;
	passwd = getpass(swascii ? "L|senord: " : "Lösenord: ");
	if (rk_login(userid, passwd)) {
		rprintf("Felaktigt lösenord.\n\n");
		return;
	}
	myuid = rv[0].rc_conf_no;

	readvars();
	/*
	 * Set some informative text.
	 */
	if (rk_whatido(getval("kom-mercial")))
		rprintf("Set what-i-am-doing sket sej\n");

	/* Show where we have unread texts. */
	if (iseql("print-number-of-unread-on-entrance", "1"))
		list_news(0);

#if defined(SOLARIS) || defined(SUNOS4)
#undef SIG_IGN
#define SIG_IGN (void(*)(int))1
#endif
	/* Check ctrl-c flag */
	if (isneq("ignore-ctrl-c", "0"))
		signal(SIGQUIT, SIG_IGN);

	rc = rk_confinfo(myuid);
	if (rc->rc_msg_of_day) {
		rprintf("Du har en lapp på dörren.\n");
		show_text(rc->rc_msg_of_day, 1);
	}

	c = rk_unreadconf(myuid);
	if (c[0])
		prompt = PROMPT_NEXT_CONF;
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
	rkom_logout();
}

void 
cmd_send(char *str)
{
	char *buf;

	rprintf("Sänd (alarmmeddelande till alla)\n");

	buf = getstr("Meddelande: ");
	if (*buf == 0)
		rprintf("Nähej.");
	else
		rk_send_msg(0, buf);
}

void 
cmd_say(char *str)
{
	struct rk_confinfo *rv;
	char *buf;

	if (str == 0) {
		rprintf("Du måste ange vem du vill skicka meddelande till.\n");
		return;
	}
	if ((rv = match_complain(str, MATCHCONF_PERSON|MATCHCONF_CONF)) == 0)
		return;

	rprintf("Sänd meddelande till %s\n", rv[0].rc_name);

	buf = getstr("Meddelande: ");
	if (*buf != 0) {
		if (rk_send_msg(rv[0].rc_conf_no, buf))
			rprintf("\nMeddelandet kunde inte skickas.\n");
		else
			rprintf("\nMeddelandet sänt till %s.\n", rv[0].rc_name);
	} else
		rprintf("Nähej.");
}

void
cmd_where(char *str)
{
	struct rk_membership *m;
	struct rk_conference *conf;

	if (myuid == 0)
		rprintf("Du är inte inloggad");
	else if (curconf == 0)
		rprintf("Du är inte närvarande någonstans");
	else {
		if ((conf = rk_confinfo(curconf)) == NULL) {
			rprintf("Du är i voiden på server %s.\n", server);
		} else {
			m = rk_membership(myuid, curconf);
			rprintf("Du är i möte %s med %d olästa inlägg.\n",
			    conf->rc_name, conf->rc_first_local_no + 
			    conf->rc_no_of_texts - 1 - m->rm_last_text_read);
		}
	}
}

void
cmd_goto(char *str)
{
	struct rk_conference *rkc;
	struct rk_membership *m;
	struct rk_confinfo *rv;
	int conf, ret;
	char *ch, *name;

	if (myuid == 0)
		return rprintf("Logga in först.\n");

	if (str == 0)
		return rprintf("Du måste ge ett möte som argument.\n");

	if ((rv = match_complain(str, MATCHCONF_CONF|MATCHCONF_PERSON)) == 0)
		return;

	conf = rv[0].rc_conf_no;
	name = rv[0].rc_name;
	ret = rk_change_conference(conf);
	if (ret == 0) {
		curconf = conf;
		rprintf("Du är nu i möte %s.\n", name);
		goto done;
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
done:
	if ((rkc = rk_confinfo(conf)) == NULL) 
		return rprintf("Kunde inte läsa confinfon för %s: %s\n",
		    name, error(komerr));
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
	if ((conf = rk_confinfo(curconf)) == NULL)
		return rprintf("Kunde inte läsa confinfon: %s\n",
		    error(komerr));
	high = conf->rc_first_local_no + conf->rc_no_of_texts - 1;
	if (only > high)
		only = high;
	rk_set_last_read(curconf, high - only);
	next_resetchain();
	next_prompt();
}

void
cmd_leave(char *str)
{
	struct rk_confinfo *rv;
	int ret;

	if ((rv = match_complain(str, MATCHCONF_CONF)) == NULL)
		return;
	ret = rk_sub_member(rv[0].rc_conf_no, myuid);
	if (ret)
		rprintf("Det sket sej: %s\n", error(ret));
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
		else
			rprintf("Lösenordet är nu ändrat.\n");
	}
	free(opass);
	free(npass1);
	free(npass2);
}

void
cmd_other_password(char *str)
{
	struct rk_confinfo *rc;
	char *opass, *npass1, *npass2;
	int rv;

	if ((rc = match_complain(str, MATCHCONF_PERSON)) == NULL)
		return;
	rprintf("Ändra andras (lösenord för) %s\n\n", rc->rc_name);
	opass = strdup(getpass("Ange ditt eget lösenord: "));
	npass1 = strdup(getpass("Ange nya lösenordet: "));
	npass2 = strdup(getpass("Ange nya lösenordet igen: "));

	if (strcmp(npass1, npass2)) {
		rprintf("Du skrev olika nya lösenord, försök igen.\n");
	} else {
		rv = rk_setpass(rc->rc_conf_no, opass, npass1);
		if (rv)
			rprintf("Det sket sej: %s\n", error(rv));
		else
			rprintf("Lösenordet är nu ändrat.\n");
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
}

static void
confstat(int mid)
{
	struct rk_conference *rcp;
	struct rk_aux_item *rai;
	char *str;
	int i, nrai;

	if ((rcp = rk_confinfo(mid)) == NULL)
		return rprintf("Kunde inte läsa confinfon: %s\n",
		    error(komerr));
	rprintf("Namn:                  %s (%d)\n", vem(mid), mid);
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
	str = bitfield2str(rcp->rc_type);
	rprintf("Mötestyp:              %s\n", str+24);
	rprintf("Supervisor:            %s\n", vem(rcp->rc_supervisor));
	rprintf("Supermöte:             %s\n", vem(rcp->rc_super_conf));

	rai = rcp->rc_aux_item.rc_aux_item_val;
	nrai = rcp->rc_aux_item.rc_aux_item_len;
	rprintf("\n");
	for (i = 0; i < nrai; i++) {
		if (rai[i].rai_tag == RAI_TAG_FAQ_TEXT)
			rprintf("Mötet har en FAQ i text %s\n",
			    rai[i].rai_data);
	}
}

void
cmd_status(char *name)
{
	struct rk_confinfo *rv;

	if ((rv = match_complain(name, MATCHCONF_CONF|MATCHCONF_PERSON)) == 0)
		return;
	rprintf("Status (för) %s\n\n", rv[0].rc_name);
	if (rv[0].rc_type & RK_CONF_TYPE_LETTERBOX)
		persstat(rv[0].rc_conf_no);
	else
		confstat(rv[0].rc_conf_no);
}

void
cmd_info_extra(int text)
{
	struct rk_text_stat *rts;
	struct rk_aux_item *rai;
	int i, nrai;

	rprintf("Tilläggsinformation (för text) %d.\n\n", text);

	if ((rts = rk_textstat(text)) == NULL) {
		rprintf("Kunde inte läsa status för inlägg %d: %s.\n",
		    text, error(komerr));
		return;
	}
	nrai = rts->rt_aux_item.rt_aux_item_len;
	rai = rts->rt_aux_item.rt_aux_item_val;
	if (nrai) {
		for (i = 0; i < nrai; i++) {
			rprintf("Nummer:\t\t%d\n", rai[i].rai_aux_no);
			rprintf("Typ:\t\t%d\n", rai[i].rai_tag);
			rprintf("Skapad av:\t%s\n", vem(rai[i].rai_creator));
			rprintf("Skapad:\t\t%s\n",
			    get_date_string(&rai[i].rai_created_at));
			rprintf("Flaggor:\t\n");
			rprintf("Arvsgräns:\t%d\n", rai[i].inherit_limit);
			rprintf("Innehåll:\t\"%s\".\n", rai[i].rai_data);
		}
	} else
		rprintf("Det finns ingen tilläggsinformation för denna text.\n");
}

void
cmd_change_name()
{
	struct rk_confinfo *rv;
	int r;
	char *name;

	rprintf("Ändra namn\n\n");

	name = getstr("Vilket namn skall ändras? ");
	if (*name == 0) {
		rprintf("Nähej.\n");
		return;
	}
	if ((rv = match_complain(name, MATCHCONF_CONF|MATCHCONF_PERSON)) == 0)
		return;
	rprintf("%s\n", rv[0].rc_name);
	name = getstr("Vad skall det ändras till? ");
	r = rk_change_name(rv[0].rc_conf_no, name);
	if (r)
		rprintf("Det gick inte: %s\n", error(r));
}

void
cmd_add_member()
{
	struct rk_confinfo *rv;
	int r, uid, mid;
	char *name, *user;

	rprintf("Addera medlem\n\n");

	name = getstr("Vem skall adderas? ");
	if (*name == 0) {
		rprintf("Nähej.\n"); 
		return;
	}
	rv = match_complain(name, MATCHCONF_PERSON);
	if (rv == NULL)
		return;
	rprintf("%s\n", rv[0].rc_name);
	user = strdup(rv[0].rc_name);
	uid = rv[0].rc_conf_no;
	name = getstr("Till vilket möte? ");
	if (*name == 0) {
		rprintf("Nähej.\n");
		free(user);
		return;
	}
	rv = match_complain(name, MATCHCONF_CONF);
	if (rv == NULL) {
		free(user);
		return;
	}
	mid = rv[0].rc_conf_no;
	r = rk_add_member(mid, uid, 100, 3, 0);
	if (r)
		rprintf("Det gick inte: %s\n", error(r));
	else
		rprintf("Person %s adderad till möte %s.\n",
		    user, rv[0].rc_name);
	free(user);
}

void
cmd_sub_member()
{
	struct rk_confinfo *retval;
	int rv, uid, mid;
	char *name;

	rprintf("Subtrahera medlem\n\n");

	name = getstr("Vem skall subtraheras? ");
	if (*name == 0) {
		rprintf("Nähej.\n"); 
		return;
	}
	retval = match_complain(name, MATCHCONF_PERSON);
	if (retval == NULL)
		return;
	rprintf("%s\n", retval[0].rc_name);
	uid = retval[0].rc_conf_no;
	name = getstr("Från vilket möte? ");
	if (*name == 0) {
		rprintf("Nähej.\n");
		return;
	}
	retval = match_complain(name, MATCHCONF_CONF);
	if (retval == NULL)
		return;
	mid = retval[0].rc_conf_no;
	rv = rk_sub_member(mid, uid);
	if (rv)
		rprintf("Det gick inte: %s\n", error(rv));
}

void
cmd_add_rcpt()
{
	struct rk_confinfo *retval;
	int rv, conf, text;
	char *name, buf[50];

	rprintf("Addera mottagare\n\n");

	name = getstr("Vilket möte skall adderas? ");
	if (*name == 0) {
		rprintf("Nähej.\n"); 
		return;
	}
	retval = match_complain(name, MATCHCONF_CONF|MATCHCONF_PERSON);
	if (retval == NULL)
		return;
	rprintf("%s\n", retval[0].rc_name);
	conf = retval[0].rc_conf_no;
	if (lasttext)
		sprintf(buf, "Till vilken text? (%d) ", lasttext);
	else
		sprintf(buf, "Till vilken text? ");
	name = getstr(buf);
	if ((*name == 0) && lasttext) {
		text = lasttext;
	} else if (strlen(name) == 0) {
		rprintf("Nähej.\n");
		return;
	} else 
		text = atoi(name);
	if (text == 0) {
		rprintf("Det var ett dåligt textnummer.\n");
		return;
	}
	rv = rk_add_rcpt(text, conf, recpt);
	if (rv)
		rprintf("Det gick inte: %s\n", error(rv));
	else
		rprintf("Text %d adderad till möte %s.\n", text,
		    retval[0].rc_name);
}

void
cmd_move_text()
{
	struct rk_confinfo *retval;
	struct rk_text_stat *ts;
	struct rk_misc_info *mi;
	char *text, buf[100];
	int i, nr, cm, nm, rv, conf;

	rprintf("Flytta (text)\n\n");

	sprintf(buf, "Vilken text skall flyttas (%d)? ", lasttext);
	text = getstr(buf);
	if (*text)
		nr = atoi(text);
	else
		nr = lasttext;
	if (nr == 0) {
		rprintf("Du måste ange ett giltigt textnummer.\n\n");
		return;
	}
	if ((ts = rk_textstat(nr)) == NULL) {
		rprintf("Text %d är inte en giltig text.\n", nr);
		return;
	}
	mi = ts->rt_misc_info.rt_misc_info_val;
	for (cm = nm = i = 0; i < ts->rt_misc_info.rt_misc_info_len; i++) {
		if (mi[i].rmi_type != recpt)
			continue;
		cm = mi[i].rmi_numeric;
		nm++;
	}
	if (nm > 1) {
		rprintf("Texten tillhör mer än ett möte.\n"
		    "Subtrahera och addera texten manuellt.\n");
		return;
	}
	text = getstr("Till vilket möte? ");
	if (*text == 0) {
		rprintf("Nähej.\n"); 
		return;
	}
	retval = match_complain(text, MATCHCONF_CONF|MATCHCONF_PERSON);
	if (retval == NULL)
		return;
	rprintf("Till %s\n", retval[0].rc_name);
	conf = retval[0].rc_conf_no;
	rv = rk_add_rcpt(nr, conf, recpt);
	if (rv) {
		rprintf("Kunde ej flytta texten: %s\n", error(rv));
		return;
	}
	rv = rk_sub_rcpt(nr, cm);
	if (rv)
		rprintf("Misslyckades ta bort texten från %s: %s\n",
		    retval[0].rc_name, error(rv));
	else
		rprintf("Texten nu flyttad till %s.\n", retval[0].rc_name);
}

static int flyttade;

static void
addrmchain(textno nr, confno fromconf, confno toconf)
{
	struct rk_text_stat *ts;
	struct rk_misc_info *mi;
	int rv, i, len;

	if ((rv = rk_sub_rcpt(nr, fromconf)))
		return; /* not posted here, don't move it */

	if ((rv = rk_add_rcpt(nr, toconf, recpt)))
		return rprintf("Kunde ej flytta texten: %s\n", error(rv));
	if ((ts = rk_textstat(nr)) == NULL)
		return; /* Can't do anything */
	mi = ts->rt_misc_info.rt_misc_info_val;
	len = ts->rt_misc_info.rt_misc_info_len;

	for (i = 0; i < len; i++) {
		if (mi[i].rmi_type == footn_in ||
		    mi[i].rmi_type == comm_in)
			addrmchain(mi[i].rmi_numeric, fromconf, toconf);
	}
	flyttade++;
}

void
cmd_move_text_chain()
{
	struct rk_confinfo *retval;
	struct rk_text_stat *ts;
	struct rk_misc_info *mi;
	char *text, buf[100];
	int i, nr, cm, nm;
	int fromconf, toconf;

	rprintf("Flytta inläggskedja\n\n");

	sprintf(buf, "Vilken är första texten i kedjan (%d)? ", lasttext);
	text = getstr(buf);
	if (*text)
		nr = atoi(text);
	else
		nr = lasttext;
	if (nr == 0) {
		rprintf("Du måste ange ett giltigt textnummer.\n\n");
		return;
	}
	if ((ts = rk_textstat(nr)) == NULL) {
		rprintf("Text %d är inte en giltig text.\n", nr);
		return;
	}
	mi = ts->rt_misc_info.rt_misc_info_val;
	for (cm = nm = i = 0; i < ts->rt_misc_info.rt_misc_info_len; i++) {
		if (mi[i].rmi_type != recpt)
			continue;
		cm = mi[i].rmi_numeric;
		nm++;
	}
	if (nm > 1) {
		rprintf("Texten tillhör mer än ett möte.\n");
		text = getstr("Från vilket möte vill du subtrahera kedjan? ");
		if (*text == 0)
			return rprintf("Nähej.\n");
		retval = match_complain(text, MATCHCONF_CONF|MATCHCONF_PERSON);
		if (retval == NULL)
			return;
		rprintf("Från %s\n", retval[0].rc_name);
		fromconf = retval[0].rc_conf_no;
	} else {
		struct rk_conference *rkc = rk_confinfo(cm);
		if (rkc == NULL)
			return rprintf("Det sket sej: %s\n", error(komerr));
		rprintf("Från %s\n", rkc->rc_name);
		fromconf = cm;
	}
	text = getstr("Till vilket möte? ");
	if (*text == 0)
		return rprintf("Nähej.\n"); 
	retval = match_complain(text, MATCHCONF_CONF|MATCHCONF_PERSON);
	if (retval == NULL)
		return;
	rprintf("Till %s\n", retval[0].rc_name);
	toconf = retval[0].rc_conf_no;

	flyttade = 0;
	addrmchain(nr, fromconf, toconf);

	if (flyttade)
		rprintf("Du flyttade %d inlägg.\n", flyttade);
	else
		rprintf("Du flyttade inga inlägg.\n");
}

void 
cmd_sub_rcpt()
{
	struct rk_confinfo *retval;
	int rv, conf, text;
	char *name, buf[50];

	rprintf("Subtrahera mottagare \n\n");

	name = getstr("Vilket möte skall subtraheras? ");
	if (*name == 0) {
		rprintf("Nähej.\n"); 
		return;
	}
	retval = match_complain(name, MATCHCONF_CONF|MATCHCONF_PERSON);
	if (retval == NULL)
		return;
	rprintf("%s\n", retval[0].rc_name);
	conf = retval[0].rc_conf_no;
	if (lasttext)
		sprintf(buf, "Från vilken text? (%d) ", lasttext);
	else
		sprintf(buf, "Från vilken text? ");
	name = getstr(buf);
	if ((*name == 0) && lasttext) {
		text = lasttext;
	} else if (strlen(name) == 0) {
		rprintf("Nähej.\n");
		return;
	} else
		text = atoi(name);
	if (text == 0) {
		rprintf("Det var ett dåligt textnummer.\n");
		return;
	}
	rv = rk_sub_rcpt(text, conf);
	if (rv)
		rprintf("Det gick inte: %s\n", error(rv));
	else
		rprintf("Text %d subtraherad från möte %s.\n", text,
		    retval[0].rc_name);
}

void
cmd_delete(int text)
{
	int rv;

	rv = rk_delete_text(text);
	if (rv)
		rprintf("Det gick inte: %s\n", error(rv));
	else
		rprintf("Text %d nu raderad.\n", text);
}

void
cmd_create(void)
{
	char *name, *ch;
	int type = 0, ret;

	name = strdup(getstr("Vad skall mötet heta? "));
	if (*name == 0) {
		rprintf("Nehej.\n");
		return;
	}
	do {
		ch = getstr("Skall mötet vara öppet eller slutet? "
		    "(öppet, slutet) - ");
	} while (strcasecmp("öppet", ch) && strcasecmp("slutet", ch));
	if (strcasecmp("slutet", ch) == 0)
		type |= RK_CONF_TYPE_RD_PROT;
	do {
		ch = getstr("Skall mötet vara hemligt? (ja, nej) -");
	} while (strcasecmp("ja", ch) && strcasecmp("nej", ch));
	if (strcasecmp("ja", ch) == 0)
		type |= RK_CONF_TYPE_SECRET;
	ret = rk_create_conf(name, type);
	if (ret < 0) {
		rprintf("Det sket sej: %s\n", error(-ret));
	} else {
		rprintf("Mötet \"%s\" är nu skapat.\n", name);
		rprintf("Glöm inte att skriva en presentation för mötet.\n");
	}
	free(name);
}

void
cmd_create_person(void)
{
	struct rk_confinfo *r;
	char *name, *npass1, *npass2;
	int i, rv;

	name = getstr("Vad skall personen heta? ");
	if (*name == 0) {
		rprintf("Nehej.\n");
		return;
	}
	r = rk_matchconf(name, MATCHCONF_PERSON);
	if (r) for (i = 0; r[i].rc_name; i++)
		if (strcasecmp(r[i].rc_name, name) == 0) {
			rprintf("Personen finns redan: %s\n", r[i].rc_name);
			return;
		}
	
	rprintf("Sätt ett lösenord för %s\n", name);
	do {
		npass1 = strdup(getpass("Ange nya lösenordet: "));
		npass2 = strdup(getpass("Ange nya lösenordet igen: "));

		if (strcmp(npass1, npass2))
			rprintf("Du skrev olika nya lösenord, försök igen.\n");
		else
			break;
	} while (1);
	rv = rk_create_person(name, npass1, 0);
	if (rv < 0) {
		rprintf("Det sket sej: %s\n", error(-rv));
	} else {
		rprintf("Personen \"%s\" är nu skapad.\n", name);
		rprintf("Glöm inte att skriva en presentation för personen.\n");
	}
	free(npass1);
	free(npass2);
}

void
cmd_erase(char *name)
{
	struct rk_confinfo *rv;
	int r;

	if ((rv = match_complain(name, MATCHCONF_CONF|MATCHCONF_PERSON)) == 0)
		return;
	r = rk_delete_conf(rv[0].rc_conf_no);
	if (r)
		rprintf("Det sket sej: %s\n", error(r));
	else
		rprintf("Raderat och klart!\n");
}

void
cmd_copy()
{
	struct rk_confinfo *retval;
	int rv, conf, text;
	char *name, buf[50];

	rprintf("(Skicka) kopia\n\n");

	name = getstr("Vart skall kopian skickas? ");
	if (*name == 0) {
		rprintf("Nähej.\n"); 
		return;
	}
	retval = match_complain(name, MATCHCONF_CONF|MATCHCONF_PERSON);
	if (retval == NULL)
		return;
	rprintf("%s\n", retval[0].rc_name);
	conf = retval[0].rc_conf_no;
	if (lasttext)
		sprintf(buf, "Vilken text skall ha en kopia? (%d) ", lasttext);
	else
		sprintf(buf, "Vilken text skall ha en kopia? ");
	name = getstr(buf);
	if ((*name == 0) && lasttext) {
		text = lasttext;
	} else if (*name == 0) {
		rprintf("Nähej.\n");
		return;
	} else 
		text = atoi(name);
	if (text == 0) {
		rprintf("Det var ett dåligt textnummer.\n");
		return;
	}
	rv = rk_add_rcpt(text, conf, cc_recpt);
	if (rv)
		rprintf("Det gick inte: %s\n", error(rv));
	else
		rprintf("Text %d adderad till %s.\n", text, retval[0].rc_name);
}

void
cmd_change_priority(char *str)
{
	struct rk_confinfo *rv;
	struct rk_membership *rm;
	char *name;
	int pri, i;

	if ((rv = match_complain(str, MATCHCONF_CONF)) == 0)
		return;
	rprintf("Ändra prioritet på möte %s\n", rv->rc_name);
	if ((rm = rk_membership(myuid, rv->rc_conf_no)) == NULL) {
		printf("Det går inte: %s\n", error(komerr));
		return;
	}
	rprintf("Tidigare prioritet: %d\n", rm->rm_priority);
	name = getstr("Vilken ny prioritet skall mötet ha? ");
	if (*name == 0) {
		rprintf("Nähej.\n");
		return;
	}
	pri = atoi(name);
	if (pri <= 0 || pri > 255) {
		rprintf("'%s' är en otillåten prioritet.\n", name);
		return;
	}
	if ((i = rk_add_member(rv->rc_conf_no, myuid, pri, 
	    rm->rm_position, rm->rm_type)) != 0)
		rprintf("Det sket sej: %s\n", error(i));
	else
		rprintf("Prioriteten nu ändrad till %d.\n", pri);
}

void
cmd_enable()
{
	int err;

	rprintf("Övergå (till administratörsmod)\n");
	if ((err = rk_enable(255)))
		rprintf("Det gick inte: %s\n", error(err));
	else
		rprintf("Du är nu administratör.\n");
}

void
cmd_disable()
{
	int err;

	rprintf("Lämna (administratörsmod)\n");
	if ((err = rk_enable(0)))
		rprintf("Det gick inte: %s\n", error(err));
	else
		rprintf("Du är nu inte administratör.\n");
}
