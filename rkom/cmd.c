
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <pwd.h>

#include "exported.h"
#include "rkom.h"
// #include "conf.h"
// #include "next.h"
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
	{"g�", 0, cmd_goto },
//	{"gl�m", 0, write_forget },
//	{"hela", 0, write_whole },
//	{"inl�gg", 0, write_new },
//	{"kommentera", 0, write_cmnt },
//	{"kommentar", "till:", write_comment },
//	{"lista", "kommandon", list_comm },
//	{"lista", "m�ten", list_conf },
//	{"lista", "nyheter", list_news },
	{"login", 0, cmd_login },
	{"logout", 0, cmd_logout },
//	{"l�gg", 0, write_put },
//	{"mottagare:", 0, write_rcpt },
//	{"n�sta", "inl�gg", next_text },
//	{"n�sta", "m�te", next_conf },
//	{"redigera", "editor", write_editor },
	{"sluta", 0, cmd_sluta },
	{"s�g", 0, cmd_say },
	{"s�nda", 0, cmd_send },
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
		printf("Ok�nt kommando: %s\n", arg1);
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
		printf("Ok�nt kommando: %s %s\n", arg1, arg2);
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

static char *dindx[] = {"m�n", "tis", "ons", "tors", "fre", "l�r", "s�n"};
static char *mindx[] = {"januari", "februari", "mars", "april", "maj", "juni",
	"juli", "augusti", "september", "oktober", "november", "december"};

static char *tindx[] = {"tolv", "ett", "tv�", "tre", "fyra", "fem", "sex",
	"sju", "�tta", "nio", "tio", "elva" };

static char *dyidx[] = {"natten", "morgonen", "eftermiddagen", "kv�llen" };

void
cmd_tiden(char *str)
{
	struct tm tm;

	rkom_time(&tm);

	printf("Klockan �r ");
	if (tm.tm_min > 30)
		printf("%d minuter i %s", 60 - tm.tm_min,
		    tindx[((tm.tm_hour + 1)%12)]);
	else if (tm.tm_min == 30)
		printf("halv %s", tindx[((tm.tm_hour+1)%12)]);
	else if (tm.tm_min == 0)
		printf("prick %s", tindx[(tm.tm_hour%12)]);
	else
		printf("%d minuter �ver %s", tm.tm_min, tindx[(tm.tm_hour%12)]);
	printf(" p� %s", dyidx[tm.tm_hour/6]);
	printf(",\n%sdagen den %d %s %d", dindx[tm.tm_wday], tm.tm_mday,
	    mindx[tm.tm_mon], tm.tm_year + 1900);
	if (tm.tm_isdst)
		printf(" (sommartid)");
	printf(" (enligt servern)\n");
}

void
cmd_vilka(char *str)
{
	int i, antal;
	struct dynamic_session_info *pp;

	rkom_who(0, WHO_VISIBLE, &pp);

	for (antal = 0; pp[antal].session; antal++)
		;

	if (antal == 0) {
		printf("Det �r inga p�loggade alls.\n");
		return;
	}

	printf("Det �r %d person%s p�loggade.\n",
	    antal, antal == 1 ? "" : "er");
	printf("-------------------------------------------------------\n");

	for (i = 0; i < antal; i++) {
		struct conference *c1, *c2;
		struct person *p;
		char *name, *conf, *var;

		rkom_confinfo(pp[i].person, &c1);
		name = c1->name;
		if (pp[i].conf) {
			rkom_confinfo(pp[i].conf, &c2);
			conf = c2->name;
		} else
			conf = "Inte n�rvarande n�gonstans";
		rkom_persinfo(pp[i].person, &p);
		var = p->username;
		if (strlen(name) > 33)
			printf("%5d %s\n%40s%s\n",
			    pp[i].session, name, "", conf);
		else
			printf("%5d %-33s %-40s\n", pp[i].session, name, conf);
		if (strlen(var) > 36)
			printf("   %s\n%40s%s\n\n", var, "", pp[i].doing);
		else
			printf("   %-37s%s\n\n", var, pp[i].doing);
		free(c1);
		if (pp[i].conf)
			free(c2);
		free(p);
	}
	printf("-------------------------------------------------------\n");
	free(pp);
}

void
cmd_login(char *str)
{
	struct membership *m;
	struct conference *confer;
	struct confinfo *matched;
	char *passwd;
	int i, num, *confs, nconf;

	if (str == 0) {
		printf("Du m�ste ange vem du vill logga in som.\n");
		return;
	}
	num = rkom_matchconf(str, MATCHCONF_PERSON, &matched);
	if (num == 0) {
		printf("Det finns ingen person som matchar \"%s\".\n", str);
		return;
	} else if (num > 1) {
		printf("Namnet \"%s\" �r flertydigt. Du kan mena:\n", str);
		for (i = 0; i < num; i++)
			printf("%s\n", matched[i].name);
		printf("\n");
		free(matched[0].name);
		free(matched);
		return;
	}
	printf("%s\n", matched[0].name);
	passwd = getpass("L�senord: ");
	if (rkom_login(matched[0].conf_no, passwd)) {
		printf("Felaktigt l�senord.\n\n");
		free(matched[0].name);free(matched);
		return;
	}
	myuid = matched[0].conf_no;

	/*
	 * Set some informative text.
	 */
	if (rkom_whatido("K�r raggeklienten (nu med login!)"))
		printf("Set what-i-am-doing sket sej\n");

	/*
	 * Get number of unread texts.
	 */
	if (rkom_unreadconf(myuid, &confs, &nconf))
		printf("rkom_unreadconf sket sej\n");
	
	/* Show where we have unread texts. */
	if (nconf) {
		for (i = 0; i < nconf; i++) {
			int hln;

			if (rkom_confinfo(confs[i], &confer)) {
				printf("%d sket sej\n", confs[i]);
				continue;
			}
			hln = confer->first_local_no + confer->no_of_texts - 1;
			if (rkom_membership(myuid, confs[i], &m)) {
				printf("%d,%d sket sej\n", confs[i], myuid);
				continue;
			}
			printf("Du har %d ol�sta inl�gg av %d i %s\n",
			    hln - m->last_text_read, hln, confer->name);
			free(confer);
			free(m);
		}
		free(confs);
		printf("\n");
		prompt = PROMPT_NEXT_CONF;
	} else {
		printf("\nDu har inga ol�sta inl�gg.\n");
	}
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
		printf("Du h�ller p� att skriva en text. ");
		printf("L�gg in eller gl�m den f�rst.\n");
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

	printf("S�nd (alarmmeddelande till alla)\nMeddelande: ");
	fflush(stdout);

	buf = get_input_string(0); /* XXX */

	if (strlen(buf) == 0) {
		printf("N�hej.");
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

	printf("S�nd meddelande till %s\nMeddelande: ", conf_num2name(id));
	fflush(stdout);

	buf = get_input_string(0); /* XXX */

	if (strlen(buf) == 0) {
		printf("N�hej.");
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
	printf("\nMeddelandet s�nt till %s.\n", str);
#endif
}

void
cmd_where(char *str)
{
#if 0
	if (myuid == 0)
		printf("Du �r inte ens inloggad.\n");
	else if (curconf == 0)
		printf("Du �r inte n�rvarande n�gonstans.\n");
	else
		printf("Du �r i m�te %s.\n", conf_num2name(curconf));
#endif
}

void
cmd_goto(char *str)
{
#if 0
	int conf, ret;
	char *ch;

	if (myuid == 0) {
		printf("Logga in f�rst.\n");
		return;
	}
	if (str == 0) {
		printf("Du m�ste ge ett m�te som argument.\n");
		return;
	}
	conf = conf_conf2num_complain(str);
	if (conf == 0)
		return;

	ret = change_conference(conf);
	if (ret == 0) {
		curconf = conf;
		printf("Du gick nu till m�te %s.\n", conf_num2name(conf));
		return;
	}
	/* XXX Check why change_conference failed; status of conf etc */
	if (ret == 13) { /* XXX should be define */
		printf("Du �r inte medlem i %s.\n", conf_num2name(conf));
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
	printf("Du �r nu medlem i %s.\n", conf_num2name(conf));
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
		printf("Du m�ste ange hur m�nga du endast vill se.\n");
		return;
	}
	only = atoi(str);
	if (isdigit(*str) == 0) {
		printf("Du m�ste ange ett giltigt antal.\n");
		return;
	}
	if (curconf == 0) {
		printf("Du �r inte i n�t m�te just nu.\n");
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
