
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pwd.h>

#include "rkom_proto.h"
#include "exported.h"
#include "rkom.h"
// #include "conf.h"
#include "next.h"
// #include "list.h"
// #include "write.h"

static void cmd_logout(char *);
static void cmd_tiden(char *);
static void cmd_vilka(char *);
static void cmd_sluta(char *);
static void cmd_send(char *);
static void cmd_say(char *);
static void cmd_where(char *);
static void cmd_goto(char *);
static void cmd_only(char *);

struct cmnd cmds[] = {
	{"endast", 0, cmd_only },
	{"gå", 0, cmd_goto },
//	{"glöm", 0, write_forget },
//	{"hela", 0, write_whole },
//	{"inlägg", 0, write_new },
//	{"kommentera", 0, write_cmnt },
//	{"kommentar", "till:", write_comment },
//	{"lista", "kommandon", list_comm },
//	{"lista", "möten", list_conf },
//	{"lista", "nyheter", list_news },
	{"login", 0, cmd_login },
	{"logout", 0, cmd_logout },
//	{"lägg", 0, write_put },
//	{"mottagare:", 0, write_rcpt },
	{"nästa", "inlägg", next_text },
	{"nästa", "möte", next_conf },
//	{"redigera", "editor", write_editor },
	{"sluta", 0, cmd_sluta },
	{"säg", 0, cmd_say },
	{"sända", 0, cmd_send },
	{"tiden", 0, cmd_tiden },
	{"var", 0, cmd_where },
 	{"vilka", 0, cmd_vilka },
};
static int ncmds = sizeof(cmds)/sizeof(cmds[0]);

int myuid = 0, curconf = 0;

/*
 * Parse the command.
 */
void
cmd_parse(str)
	char *str;
{
	struct cmnd *lastmatch, *l2match;
	int i, nmatch;
	char *arg1, *arg2;

	/*
	 * First; try to match the command as a first-level command.
	 */
	arg1 = strsep(&str, " \t");

	l2match = 0;
	for (i = nmatch = 0; i < ncmds; i++)
		if (bcmp(cmds[i].arg1, arg1, strlen(arg1)) == 0) {
			lastmatch = &cmds[i];
			nmatch++;
			/* QUIRK QUIRK QUIRK */
			/* Do special matching for some commands. */

			if (bcmp(arg1, "kommentera", strlen(arg1)) == 0 &&
			    (str == 0 || atoi(str) > 0))
				break; /* Match on only 'k' if necessary */

			/* END QUIRK QUIRK QUIRK */
		}

	if (nmatch == 0) {
		printf("Okänt kommando: %s\n", arg1);
		return;
	}
	if (nmatch == 1) {
		(*lastmatch->func)(str);
		return;
	}

	/*
	 * It was not a first-level command. Try second-level.
	 */
	arg2 = strsep(&str, " \t");
	if (arg2 == 0) {
		printf("Flertydigt kommando. Du kan mena:\n\n");
		for (i = 0; i < ncmds; i++)
			if (bcmp(cmds[i].arg1, arg1, strlen(arg1)) == 0)
				printf("%s %s\n", cmds[i].arg1, 
				    cmds[i].arg2 ? cmds[i].arg2 : "");
		printf("\n");
		return;
	}


	for (i = nmatch = 0; i < ncmds; i++)
		if (bcmp(cmds[i].arg1, arg1, strlen(arg1)) == 0)
			if (cmds[i].arg2 && bcmp(cmds[i].arg2, arg2, 
			    strlen(arg2)) == 0) {
				lastmatch = &cmds[i];
				nmatch++;
			}

	if (nmatch == 0) {
		printf("Okänt kommando: %s %s\n", arg1, arg2);
		return;
	}
	if (nmatch == 1) {
		(*lastmatch->func)(str);
		return;
	}
	printf("Flertydigt kommando. Du kan mena:\n\n");
	for (i = 0; i < ncmds; i++)
		if ((bcmp(cmds[i].arg1, arg1, strlen(arg1)) == 0) &&
		    cmds[i].arg2 &&
		    (bcmp(cmds[i].arg2, arg2, strlen(arg2)) == 0))
			printf("%s %s\n", cmds[i].arg1, cmds[i].arg2);
	printf("\n");
};

static char *dindx[] = {"mån", "tis", "ons", "tors", "fre", "lör", "sön"};
static char *mindx[] = {"januari", "februari", "mars", "april", "maj", "juni",
	"juli", "augusti", "september", "oktober", "november", "december"};

static char *tindx[] = {"tolv", "ett", "två", "tre", "fyra", "fem", "sex",
	"sju", "åtta", "nio", "tio", "elva" };

static char *dyidx[] = {"natten", "morgonen", "eftermiddagen", "kvällen" };

void
cmd_tiden(char *str)
{
	struct rk_time *tm;

	tm = rk_time(0);

	printf("Klockan är ");
	if (tm->rt_minutes > 30)
		printf("%d minuter i %s", 60 - tm->rt_minutes,
		    tindx[((tm->rt_hours + 1)%12)]);
	else if (tm->rt_minutes == 30)
		printf("halv %s", tindx[((tm->rt_hours+1)%12)]);
	else if (tm->rt_minutes == 0)
		printf("prick %s", tindx[(tm->rt_hours%12)]);
	else
		printf("%d minuter över %s", tm->rt_minutes,
		    tindx[(tm->rt_hours%12)]);
	printf(" på %s", dyidx[tm->rt_hours/6]);
	printf(",\n%sdagen den %d %s %d", dindx[tm->rt_day_of_week], tm->rt_day,
	    mindx[tm->rt_month], tm->rt_year + 1900);
	if (tm->rt_is_dst)
		printf(" (sommartid)");
	printf(" (enligt servern)\n");
}

void
cmd_vilka(char *str)
{
	struct rk_dynamic_session_info_retval *ppp;
	struct rk_dynamic_session_info *pp;
	int i, antal;

	ppp = rk_vilka(0, WHO_VISIBLE);

	antal = ppp->rdv_rds.rdv_rds_len;
	pp = ppp->rdv_rds.rdv_rds_val;
	if (antal == 0) {
		printf("Det är inga påloggade alls.\n");
		return;
	}

	printf("Det är %d person%s påloggade.\n",
	    antal, antal == 1 ? "" : "er");
	printf("-------------------------------------------------------\n");

	for (i = 0; i < antal; i++) {
		struct rk_conference *c1, *c2;
		struct rk_person *p;
		char *name, *conf, *var;

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
			printf("%5d %s\n%40s%s\n",
			    pp[i].rds_session, name, "", conf);
		else
			printf("%5d %-33s %-40s\n",
			    pp[i].rds_session, name, conf);
		if (strlen(var) > 36)
			printf("   %s\n%40s%s\n\n", var, "", pp[i].rds_doing);
		else
			printf("   %-37s%s\n\n", var, pp[i].rds_doing);
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
	int i, num, *confs, nconf, userid;
	char *passwd;

	if (str == 0) {
		printf("Du måste ange vem du vill logga in som.\n");
		return;
	}
	retval = rk_matchconf(str, MATCHCONF_PERSON);

	num = retval->rcr_ci.rcr_ci_len;
	if (num == 0) {
		printf("Det finns ingen person som matchar \"%s\".\n", str);
		return;
	} else if (num > 1) {
		printf("Namnet \"%s\" är flertydigt. Du kan mena:\n", str);
		for (i = 0; i < num; i++)
			printf("%s\n", retval->rcr_ci.rcr_ci_val[i].rc_name);
		printf("\n");
		free(retval);
		return;
	}
	printf("%s\n", retval->rcr_ci.rcr_ci_val[0].rc_name);
	userid = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	passwd = getpass("Lösenord: ");
	if (rk_login(userid, passwd)) {
		printf("Felaktigt lösenord.\n\n");
		free(retval);
		return;
	}
	myuid = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	free(retval);

	/*
	 * Set some informative text.
	 */
	if (rk_whatido("Kör raggeklienten (nu med login!)"))
		printf("Set what-i-am-doing sket sej\n");

	/*
	 * Get number of unread texts.
	 */
	conf = rk_unreadconf(myuid);

	/* Show where we have unread texts. */
	nconf = conf->ru_confs.ru_confs_len;
	if (nconf) {
		confs = conf->ru_confs.ru_confs_val;

		for (i = 0; i < nconf; i++) {
			struct rk_conference *rkc;
			struct rk_membership *m;
			int hln;

			rkc = rk_confinfo(confs[i]);

			if (rkc->rc_retval) {
				printf("%d sket sej med %d\n",
				    confs[i], rkc->rc_retval);
				free(rkc);
				continue;
			}
			hln = rkc->rc_first_local_no + rkc->rc_no_of_texts - 1;
			m = rk_membership(myuid, confs[i]);

			if (m->rm_retval) {
				printf("%d,%d sket sej med %d\n",
				    confs[i], myuid, m->rm_retval);
				free(rkc);
				free(m);
				continue;
			}
			printf("Du har %d olästa inlägg av %d i %s\n",
			    hln - m->rm_last_text_read, hln, rkc->rc_name);
			free(rkc);
			free(m);
		}
		printf("\n");
		prompt = PROMPT_NEXT_CONF;
	} else {
		printf("\nDu har inga olästa inlägg.\n");
	}
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
#if 0
	if (is_writing) {
		printf("Du håller på att skriva en text. ");
		printf("Lägg in eller glöm den först.\n");
		return;
	}
#endif
	printf("Nu avslutar du rkom.\n");
	exit(0);
}

static void 
cmd_send(char *str)
{
#if 0
	int i;
	char *buf, *dst;

	printf("Sänd (alarmmeddelande till alla)\nMeddelande: ");
	fflush(stdout);

	buf = get_input_string(0); /* XXX */

	if (strlen(buf) == 0) {
		printf("Nähej.");
		free(buf);
		return;
	}
	dst = malloc(strlen(buf) + 10);
	sprintf(dst, "53 0 %ldH%s\n", (long)strlen(buf), buf);
	free(buf);
	if (send_reply(dst)) {
		i = get_int();
		if (i)
			printf("%s\n", error(i));
		get_eat('\n');
		return;
	}
	get_eat('\n');
#endif
}

void 
cmd_say(char *str)
{
#if 0
	int i, id;
	char *buf, *dst;

	id = conf_any2num_complain(str);
	if (id == 0)
		return;

	printf("Sänd meddelande till %s\nMeddelande: ", conf_num2name(id));
	fflush(stdout);

	buf = get_input_string(0); /* XXX */

	if (strlen(buf) == 0) {
		printf("Nähej.");
		free(buf);
		return;
	}
	dst = malloc(strlen(buf) + 10);
	sprintf(dst, "53 %d %ldH%s\n", id, (long)strlen(buf), buf);
	free(buf);
	if (send_reply(dst)) {
		i = get_int();
		if (i)
			printf("%s\n", error(i));
		get_eat('\n');
		return;
	}
	get_eat('\n');
	printf("\nMeddelandet sänt till %s.\n", str);
#endif
}

void
cmd_where(char *str)
{
#if 0
	if (myuid == 0)
		printf("Du är inte ens inloggad.\n");
	else if (curconf == 0)
		printf("Du är inte närvarande någonstans.\n");
	else
		printf("Du är i möte %s.\n", conf_num2name(curconf));
#endif
}

void
cmd_goto(char *str)
{
#if 0
	int conf, ret;
	char *ch;

	if (myuid == 0) {
		printf("Logga in först.\n");
		return;
	}
	if (str == 0) {
		printf("Du måste ge ett möte som argument.\n");
		return;
	}
	conf = conf_conf2num_complain(str);
	if (conf == 0)
		return;

	ret = change_conference(conf);
	if (ret == 0) {
		curconf = conf;
		printf("Du gick nu till möte %s.\n", conf_num2name(conf));
		return;
	}
	/* XXX Check why change_conference failed; status of conf etc */
	if (ret == 13) { /* XXX should be define */
		printf("Du är inte medlem i %s.\n", conf_num2name(conf));
		do {
			printf("Vill du bli medlem? (ja, nej) - ");
			fflush(stdout);
			ch = get_input_string(0); /* XXX */
		} while (strcasecmp("ja", ch) && strcasecmp("nej", ch));
		if (strcasecmp("nej", ch) == 0) {
			printf("Nehepp.\n");
			return;
		}
	} else if (ret != 0) {
		printf("%s\n", error(ret));
		return;
	}
	if ((ret = add_member(conf, myuid, 100, 3, 0))) {
		printf("%s\n", error(ret));
		return;
	}
	curconf = conf;
	printf("Du är nu medlem i %s.\n", conf_num2name(conf));
	delete_membership_internal();
	ret = get_uconf_stat(curconf)->highest_local_no -
	    get_membership(myuid, curconf)->last_text_read;
	if (ret) {
		printf("\nDu har %d ol\xe4sta inl\xe4gg.\n", ret);
		prompt = PROMPT_NEXT_TEXT;
	} else {
		printf("\nDu har inga ol\xe4sta inl\xe4gg.\n");
		prompt = PROMPT_NEXT_CONF;
	}
#endif
}

void
cmd_only(char *str)
{
#if 0
	int only, high;
	char buf[20];

	if (str == 0) {
		printf("Du måste ange hur många du endast vill se.\n");
		return;
	}
	only = atoi(str);
	if (isdigit(*str) == 0) {
		printf("Du måste ange ett giltigt antal.\n");
		return;
	}
	if (curconf == 0) {
		printf("Du är inte i nåt möte just nu.\n");
		return;
	}
	high = get_uconf_stat(curconf)->highest_local_no;
	sprintf(buf, "77 %d %d\n", curconf, high - only);
	if (send_reply(buf)) {
		printf("%s\n", error(get_int()));
		get_eat('\n');
		return;
	}
	get_eat('\n');
	set_last_read_internal(curconf, high - only);
	prompt = PROMPT_NEXT_CONF;
#endif
}
