/* $Id: parse.c,v 1.36 2001/12/01 14:43:08 ragge Exp $ */

#include <sys/param.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rkomsupport.h"
#include "next.h"
#include "write.h"

void cmd_logout(char *);
void cmd_login(char *);
void cmd_tiden(char *);
void cmd_vilka(char *);
void cmd_sluta(char *);
void cmd_send(char *);
void cmd_say(char *);
void cmd_where(char *);
void cmd_goto(char *);
void cmd_only(char *);
void cmd_leave(char *);

#include "list.h"
#include "rkom.h"
#include "set.h"
#include "rkom_proto.h"


#include "parse_eng.h"
#include "parse.h"

#if defined(__STDC__) || defined(__cplusplus)
#	define CC_2(a,b)	a ## b
#else
#	define CC_2(a,b) a/**/b
#endif

#define DCMD(x) \
static int CC_2(exec_,x) (int argc, char *argv[])

/* Commands for reading */
DCMD(read_next_text);
DCMD(read_next_cmt);
DCMD(read_see_again_cmt);
DCMD(read_see_again_cmt_no);
DCMD(read_see_presentation);
DCMD(read_see_unmodified);
DCMD(read_see_again_root);
DCMD(read_list_news);
DCMD(read_only);
DCMD(read_again);
DCMD(read_jump);

/* Commands for writing */
DCMD(write_new);
DCMD(write_cmt);
DCMD(write_cmt_no);
DCMD(write_cmt_last);
DCMD(write_footnote);
DCMD(write_footnote_no);
DCMD(write_letter);
DCMD(write_private);

/* Commands for conferences */
DCMD(conf_add_member);
DCMD(conf_sub_member);
DCMD(conf_where);
DCMD(conf_goto);
DCMD(conf_goto_next);
DCMD(conf_list);
DCMD(conf_leave);
DCMD(conf_change_presentation);
DCMD(conf_create);
DCMD(conf_delete);

/* Commands for texts */
DCMD(text_add_rcpt_late);
DCMD(text_sub_rcpt_late);
DCMD(text_put);
DCMD(text_add_rcpt);
DCMD(text_add_copy);
DCMD(text_add_cmt_to);
DCMD(text_forget);
DCMD(text_show);
DCMD(text_edit);
DCMD(text_mark);
DCMD(text_unmark);
DCMD(text_list_marked);
DCMD(text_save);
DCMD(text_list_subject);
DCMD(text_list_unread);
DCMD(text_delete);
DCMD(text_copy);
DCMD(text_move);

/* Commands for online communication */
DCMD(com_who);
DCMD(com_send);
DCMD(com_say);

/* Commands for information */
DCMD(info_list_commands);
DCMD(info_flags);
DCMD(info_time);
DCMD(info_saveflags);
DCMD(info_status);
DCMD(info_extra);
DCMD(info_set_motd);
DCMD(info_remove_motd);

/* Commands for aliases */
DCMD(alias_add);
DCMD(alias_delete);
DCMD(alias_list);


/* Other commands */
DCMD(other_set);
DCMD(other_login);
DCMD(other_logout);
DCMD(other_quit);
DCMD(other_password);
DCMD(other_name);
DCMD(other_sync);
DCMD(other_exec);
DCMD(other_create_person);

#if 1
/* Debug help */
DCMD(debug_show_args);
#endif

struct command_list {
	char	*cl_str;
	int		cl_prio;
	int		cl_takes_arg;
	cmd_func_t cl_exec;
};

#define DROW(name, prio, takes_arg, command) \
{ name, prio, takes_arg, CC_2(exec_,command) },

/*
 * Note that two commands with the same first letter shouldn't use the
 * same value of prio. Prio is an int so that shouldn't be a problem.
 */
struct command_list commands[] = {
/* Commands for reading */
DROW("nästa inlägg",			0,PE_NO_ARG,read_next_text)
DROW("nästa kommentar",			0,PE_NO_ARG,read_next_cmt)
DROW("återse",				0,PE_NUM_ARG,read_see_again_cmt_no)
DROW("återse kommenterade",		1,PE_NO_ARG,read_see_again_cmt)
DROW("återse oredigerat",		0,PE_NUM_ARG,read_see_unmodified)
DROW("återse presentation",		0,PE_STR_ARG,read_see_presentation)
DROW("återse urinlägg",			0,PE_NO_ARG,read_see_again_root)
DROW("lista nyheter",			0,PE_NO_ARG,read_list_news)
DROW("endast",					1,PE_NUM_ARG,read_only)
DROW("igen",					0,PE_NO_ARG,read_again)
DROW("hoppa",					0,PE_NO_ARG,read_jump)

/* Commands for writing */
DROW("inlägg",					0,PE_NO_ARG,write_new)
DROW("kommentera",				0,PE_NUM_ARG,write_cmt_no)
DROW("kommentera",				1,PE_NO_ARG,write_cmt)
DROW("kommentera föregånde",			0,PE_NO_ARG,write_cmt_last)
DROW("fotnot",					0,PE_NUM_ARG,write_footnote_no)
DROW("fotnot",					1,PE_NO_ARG,write_footnote)
DROW("brev",					0,PE_STR_ARG,write_letter)
DROW("personligt",				0,PE_STR_ARG,write_private)

/* Commands for conferences */
DROW("addera medlem",				0,PE_NO_ARG,conf_add_member)
DROW("subtrahera medlem",			0,PE_NO_ARG,conf_sub_member)
DROW("var",					0,PE_NO_ARG,conf_where)
DROW("gå",					0,PE_STR_ARG,conf_goto)
DROW("nästa möte",				0,PE_NO_ARG,conf_goto_next)
DROW("lista möten",				0,PE_NO_ARG,conf_list)
DROW("utträda",					0,PE_STR_ARG,conf_leave)
DROW("ändra presentation",		0,PE_STR_ARG,conf_change_presentation)
DROW("skapa möte",				0,PE_NO_ARG,conf_create)
DROW("radera möte",				0,PE_STR_ARG,conf_delete)

/* Commands for texts */
DROW("addera mottagare",			0,PE_NO_ARG,text_add_rcpt_late)
DROW("subtrahera mottagare",			0,PE_NO_ARG,text_sub_rcpt_late)
DROW("lägga",					0,PE_NO_ARG,text_put)
DROW("mottagare:",				0,PE_STR_ARG,text_add_rcpt)
DROW("extrakopia:",				0,PE_STR_ARG,text_add_copy)
DROW("kommentar till:",			0,PE_NUM_ARG,text_add_cmt_to)
DROW("glöm",					0,PE_NO_ARG,text_forget)
DROW("hela",					0,PE_NO_ARG,text_show)
DROW("redigera",				1,PE_NO_ARG,text_edit)
DROW("markera",					0,PE_NUM_ARG,text_mark)
DROW("avmarkera",				0,PE_NUM_ARG,text_unmark)
DROW("lista markerade",			0,PE_NO_ARG,text_list_marked)
DROW("spara",					0,PE_STR_ARG,text_save)
DROW("spara flaggor",				0,PE_NO_ARG,info_saveflags)
DROW("lista ärenden",			0,PE_NO_ARG,text_list_subject)
DROW("lista olästa",			0,PE_NO_ARG,text_list_unread)
DROW("radera",					0,PE_NUM_ARG,text_delete)
DROW("kopia",					0,PE_NO_ARG,text_copy)
DROW("flytta",					0,PE_NO_ARG,text_move)

/* Commands for online communication */
DROW("vilka",					0,PE_STR_ARG,com_who)
DROW("sänd",					0,PE_NO_ARG,com_send)
DROW("säg",						0,PE_STR_ARG,com_say)

/* Commands for information */
DROW("lista kommandon",			0,PE_NO_ARG,info_list_commands)
DROW("flaggor",					0,PE_NO_ARG,info_flags)
DROW("tiden",					0,PE_NO_ARG,info_time)
DROW("hjälp",					0,PE_NO_ARG,info_list_commands)
DROW("?",					0,PE_NO_ARG,info_list_commands)
DROW("status",					0,PE_STR_ARG,info_status)
DROW("tillägsinformation",			0,PE_NUM_ARG,info_extra)
DROW("lapp", 					0,PE_STR_ARG,info_set_motd)
DROW("ta",					0,PE_STR_ARG,info_remove_motd)

/* Commands for aliases */
DROW("alias",					0,PE_STR_ARG,alias_add)
DROW("unalias",					0,PE_STR_ARG,alias_delete)
DROW("lista alias",				0,PE_NO_ARG,alias_list)

/* Other commands */
DROW("sätt",					0,PE_STR_ARG,other_set)
DROW("login",					1,PE_STR_ARG,other_login)
DROW("logout",					0,PE_NO_ARG,other_logout)
DROW("radera person",				0,PE_STR_ARG,conf_delete)
DROW("skapa person",				0,PE_NO_ARG,other_create_person)
DROW("sluta",					0,PE_NO_ARG,other_quit)
DROW("synkronisera",				0,PE_NO_ARG,other_sync)
DROW("ändra lösenord",				0,PE_NO_ARG,other_password)
DROW("ändra namn",				0,PE_NO_ARG,other_name)
DROW("!", 					0,PE_STR_ARG,other_exec)

#if 1
/* Debug help */
DROW("showarg",					0,PE_STR_ARG,debug_show_args)
#endif

/* Terminate list */
{NULL,0,0,NULL}
};


#define CHK_INIT \
do { \
	if (!inited) { \
		inited = 1; \
		p_init(); \
	} \
} while (0)

static cmds_t *cmds;
static int inited = 0;

static void p_init(void);

static void
p_init(void)
{
	struct command_list *c;

	c = commands;
	cmds = parse_new_cmds();
	for (c = commands; c->cl_str != NULL; c++)
		parse_add_cmd(cmds, c->cl_str, c->cl_prio,
		    c->cl_takes_arg, c->cl_exec);
}

static char *cvtstr[] = {
	"åÅ}]", "äÄ{[", "öÖ|\\", 
};

static void
chrconvert(char *str)
{
	int len = strlen(str);
	int i, j;

	for (i = 0; i < len; i++) {
		for (j = 0; j < sizeof(cvtstr)/sizeof(char *); j++) {
			if (cvtstr[j][2] == str[i])
				str[i] = cvtstr[j][0];
			else if (cvtstr[j][3] == str[i])
				str[i] = cvtstr[j][1];
		}
#if 0
		if (str[i] > 0)
			str[i] = tolower(str[i]);
#endif
	}
}

int
exec_cmd(const char *str)
{
	char *arg;
	int rv;

	CHK_INIT;

	if (strlen(str) == 0) {
		if (prompt == PROMPT_SEE_TIME)
			cmd_tiden(NULL);
		else if (prompt == PROMPT_NEXT_TEXT)
			next_text(NULL);
		else if (prompt == PROMPT_NEXT_CONF)
			next_conf(NULL);
		else if (prompt == PROMPT_NEXT_COMMENT)
			next_comment(NULL);
		else
			rprintf("Okänd prompt %s\n", prompt);
		return 0;
	}
	arg = strdup(str);
	chrconvert(arg);
	rv = parse_exec(cmds, arg);
	free(arg);
	return rv;
}

/* Helpers */
#define XXTEST(name, test, text) \
static int name(void) {if(test)rprintf(text);else return 0;return 1;}
#define TT(cmp, str) if (cmp) {rprintf(str);return 0;}

XXTEST(nwa, is_writing, "Du håller redan på att skriva en text.\n")
#define NWA if (nwa()) return 0
XXTEST(owa, is_writing == 0, "Du skriver ingen text just nu.\n")
#define OWA if (owa()) return 0
XXTEST(lf, myuid == 0, "Du måste logga in först.\n")
#define LF if (lf()) return 0
XXTEST(mhc, curconf == 0, "Du måste gå till ett möte först.\n")
#define MHC if (mhc()) return 0

/* re-concat parameters again */
static char *
re_concat(int argc, char *argv[])
{
	int i, tot;
	static char *ret;

	if (ret)
		free(ret);
	for (i = tot = 0; i < argc; i++)
		tot += strlen(argv[i]);
	tot += argc + 10;
	ret = calloc(tot, 1);
	for (i = 0; i < argc; i++) {
		strcat(ret, argv[i]);
		strcat(ret, " ");
	}
	if (argc)
		ret[strlen(ret) - 1] = 0;
	return ret;
}

/** The actual implementation of the exec_XXX functions declared earlier **/

#define P_CALL_INFO \
do { \
	int i; \
 \
	rprintf("%s: ", __FUNCTION__); \
	for (i = 0; i < argc; i++) \
		rprintf("%s ", argv[i]); \
	rprintf("\n"); \
} while (0)

/* Commands for reading */
static int
exec_read_next_text(int argc, char *argv[])
{
	LF;
	next_text(NULL);
	return 0;
}

static int
exec_read_next_cmt(int argc, char *argv[])
{
	LF;
	next_comment(NULL);
	return 0;
}

static int
exec_read_see_again_cmt_no(int argc, char *argv[])
{
	LF;
	if (argc != 1) {
		rprintf("Handhavande: återse <inläggsnummer>\n");
		return 0;
	}
	next_resee_text(atoi(argv[0]));
	return 0;
}

static int
exec_read_see_unmodified(int argc, char *argv[])
{
	int textno;

	LF;
	if (argc > 1) {
		rprintf("Handhavande: återse oredigerat <inläggsnummer>\n");
		return 0;
	} else if (argc == 1)
		textno = atoi(argv[0]);
	else
		textno = lasttext;
	next_resee_text_unmodified(textno);
	return 0;
}

static int
exec_read_see_again_cmt(int argc, char *argv[])
{
	LF;
	next_resee_comment();
	return 0;
}

static int
exec_read_see_again_root(int argc, char *argv[])
{
	LF;
	next_resee_root(lasttext);
	return 0;
}

static int
exec_read_list_news(int argc, char *argv[])
{
	LF;
	list_news(0);
	return 0;
}

static int
exec_read_only(int argc, char *argv[])
{
	MHC;
	TT(argc < 1, "Du måste ange hur många du endast vill se.\n");
	cmd_only(argv[0]);
	return 0;
}

static int
exec_read_again(int argc, char *argv[])
{
	next_again(NULL);
	return 0;
}

static int
exec_read_jump(int argc, char *argv[])
{
	next_hoppa(NULL);
	return 0;
}


/* Commands for writing */
static int
exec_write_new(int argc, char *argv[])
{
	LF;
	MHC;
	write_new(NULL);
	return 0;
}

static int
exec_write_cmt(int argc, char *argv[])
{
	LF;
	NWA;
	write_cmnt();
	return 0;
}

static int
exec_write_cmt_no(int argc, char *argv[])
{
	LF;
	NWA;
	write_cmnt_no(atoi(argv[0]));
	return 0;
}

static int
exec_write_cmt_last(int argc, char *argv[])
{
	LF;
	NWA;
	write_cmnt_last();
	return 0;
}

static int
exec_write_footnote_no(int argc, char *argv[])
{
	LF;
	NWA;
	write_footnote_no(atoi(argv[0]));
	return 0;
}

static int
exec_write_footnote(int argc, char *argv[])
{
	LF;
	NWA;
	write_footnote();
	return 0;
}

static int
exec_write_letter(int argc, char *argv[])
{
	LF;
	NWA;
	TT(argc < 1, "Du måste ange mottagare.\n");
	write_brev(re_concat(argc, argv));
	return 0;
}

static int
exec_write_private(int argc, char *argv[])
{
	LF;
	NWA;
	TT((argc == 0) && (lasttext == 0),
	    "Det finns ingen text att svara personligt på.\n");
	TT((argc == 1) && (atoi(argv[0]) == 0),
	    "Du måste ange ett riktigt textnummer som argument.\n");
	TT(argc > 1, "\"Personligt (svar)\" tar max ett argument.\n");
	write_private(argc == 0 ? lasttext : atoi(argv[0]));
	return 0;
}


/* Commands for conferences */
static int
exec_conf_where(int argc, char *argv[])
{
	LF;
	cmd_where(NULL);
	return 0;
}

static int
exec_conf_goto(int argc, char *argv[])
{
	LF;
	TT(argc == 0, "Handhavande:\ngå <mötesnamn>\n");
	cmd_goto(re_concat(argc, argv));
	return 0;
}

static int
exec_conf_goto_next(int argc, char *argv[])
{
	next_conf(NULL);
	return 0;
}

static int
exec_conf_list(int argc, char *argv[])
{
	list_conf(NULL);
	return 0;
}

static int
exec_conf_leave(int argc, char *argv[])
{
	LF;
	TT(argc < 1, "Handhavande:\nutträda <mötesnamn>\n");
	cmd_leave(re_concat(argc, argv));
	return 0;
}


/* Commands for texts */
static int
exec_text_put(int argc, char *argv[])
{
	OWA;
	write_put(NULL);
	return 0;
}

static int
exec_text_add_copy(int argc, char *argv[])
{
	OWA;
	TT(argc < 1, "Handhavande:\nextrakopia: <mottagarnamn>\n");
	write_rcpt(re_concat(argc, argv), cc_recpt);
	return 0;
}

static int
exec_text_add_rcpt(int argc, char *argv[])
{
	OWA;
	TT(argc < 1, "Handhavande:\nmottagare: <mottagarnamn>\n");
	write_rcpt(re_concat(argc, argv), recpt);
	return 0;
}

static int
exec_text_add_cmt_to(int argc, char *argv[])
{
	OWA;
	TT(argc < 1, "Handhavande:\nkommentar till: <ärendenummer>\n");
	write_comment(re_concat(argc, argv));
	return 0;
}

static int
exec_text_forget(int argc, char *argv[])
{
	OWA;
	write_forget(NULL);
	return 0;
}

static int
exec_text_show(int argc, char *argv[])
{
	OWA;
	write_whole(NULL);
	return 0;
}

static int
exec_text_edit(int argc, char *argv[])
{
	OWA;
	write_editor(NULL);
	return 0;
}

static int
exec_text_mark(int argc, char *argv[])
{
	LF;
	TT(argc != 1, "Handhavande:\nmarkera <ärendenummer>\n");
	list_mark(argv[0]);
	return 0;
}

static int
exec_text_unmark(int argc, char *argv[])
{
	LF;
	TT(argc != 1, "Handhavande:\navmarkera <ärendenummer>\n");
	list_unmark(argv[0]);
	return 0;
}

static int
exec_text_list_subject(int argc, char *argv[])
{
	LF;
	MHC;
	list_subject();
	return 0;
}

static int
exec_text_list_unread(int argc, char *argv[])
{
	LF;
	MHC;
	list_unread();
	return 0;
}

static int
exec_text_list_marked(int argc, char *argv[])
{
	LF;
	list_marked(NULL);
	return 0;
}

static int
exec_text_save(int argc, char *argv[])
{
	TT(argc != 1, "Handhavande:\nspara <filnamn>\n");
	show_savetext(argv[0]);
	return 0;
}


/* Commands for online communication */
static int
exec_com_who(int argc, char *argv[])
{
	cmd_vilka(re_concat(argc, argv));
	return 0;
}

static int
exec_com_send(int argc, char *argv[])
{
	LF;
	cmd_send(NULL);
	return 0;
}

static int
exec_com_say(int argc, char *argv[])
{
	LF;
	TT(argc < 1, "Handhavande:\nsäg <mottagare>\n");
	cmd_say(re_concat(argc, argv));
	return 0;
}


/* Commands for information */
static int
exec_info_list_commands(int argc, char *argv[])
{
	list_comm(NULL);
	return 0;
}

static int
exec_info_saveflags(int argc, char *argv[])
{
	LF;
	set_saveflags();
	return 0;
}

static int
exec_info_flags(int argc, char *argv[])
{
	LF;
	set_flags();
	return 0;
}

static int
exec_info_time(int argc, char *argv[])
{
	cmd_tiden(NULL);
	return 0;
}

static int
exec_info_status(int argc, char *argv[])
{
	cmd_status(re_concat(argc, argv));
	return 0;
}

/* Commands for aliases */

static int
exec_alias_add(int argc, char *argv[])
{
	TT(argc <= 1, "Handhavande:\nalias <alias> <command>\n");
	parse_add_alias(cmds, argc, argv);
	return 0;
}

static int
exec_alias_delete(int argc, char *argv[])
{
	TT(argc != 1, "Handhavande:\nunalias <alias>\n");
	parse_del_alias(cmds, argv[0]);
	return 0;
}

static int
exec_alias_list(int argc, char *argv[])
{
	parse_list_alias(cmds);
	return 0;
}


/* Other commands */
static int
exec_other_set(int argc, char *argv[])
{
	LF;
	TT(argc < 2, "Handhavande:\nsätt <flaggnamn> <värde>\n");
	set_setflag(argv[0], re_concat(argc-1, &argv[1]));
	return 0;
}

static int
exec_other_login(int argc, char *argv[])
{
	TT(argc == 0, "Handhavande:\nlogin <namn>\n");
	cmd_login(re_concat(argc, argv));
	return 0;
}

static int
exec_other_logout(int argc, char *argv[])
{
	LF;
	cmd_logout(NULL);
	return 0;
}

static int
exec_other_quit(int argc, char *argv[])
{
	cmd_sluta(NULL);
	return 0;
}

static int
exec_other_password(int argc, char *argv[])
{
	LF;
	cmd_password();
	return 0;
}

static int
exec_read_see_presentation(int argc, char *argv[])
{
	LF;
	TT(argc == 0, "Du måste ange vem du vill se presentationen för.\n");

	next_resee_presentation(re_concat(argc, argv));
	return 0;
}

static int
exec_debug_show_args(int argc, char *argv[])
{
	int		i;

	printf("argc = %d\n", argc);
	for (i = 0; i < argc; i++)
		printf("argv[%d] = '%s'\n", i, argv[i]);
	return 0;
}

static int
exec_info_extra(int argc, char *argv[])
{
	int text = lasttext;

	LF;
	TT(argc > 1, "Du kan max ange ett argument.\n");
	if (argc)
		text = atoi(argv[0]);
	TT(argc == 0 && text == 0, "Du måste ange en text.\n");
	cmd_info_extra(text);
	return 0;
}

static int
exec_other_name(int argc, char *argv[])
{
	LF;
	TT(argc != 0, "Du kan inte ange några argument.\n");
	cmd_change_name();
	return 0;
}

static int
exec_conf_add_member(int argc, char *argv[])
{
	LF;
	TT(argc != 0, "Du kan inte ange några argument.\n");
	cmd_add_member();
	return 0;
}

static int
exec_conf_sub_member(int argc, char *argv[])
{
	LF;
	TT(argc != 0, "Du kan inte ange några argument.\n");
	cmd_sub_member();
	return 0;
}

static int
exec_text_add_rcpt_late(int argc, char *argv[])
{
	LF;
	TT(argc != 0, "Du kan inte ange några argument.\n");
	cmd_add_rcpt();
	return 0;
}

static int
exec_text_sub_rcpt_late(int argc, char *argv[])
{
	LF;
	TT(argc != 0, "Du kan inte ange några argument.\n");
	cmd_sub_rcpt();
	return 0;
}

static int
exec_conf_change_presentation(int argc, char *argv[])
{
	LF;
	NWA;
	TT(argc == 0, "Du måste ange vad du vill ändra presentationen för.\n");
	write_change_presentation(re_concat(argc, argv));
	return 0;
}

static int
exec_text_delete(int argc, char *argv[])
{
	LF;

	TT(argc == 0, "Du måste ange vilket textnummer du vill radera.\n");
	TT(argc > 1, "Radera tar bara ett textnummer som argument.\n");
	TT(atoi(argv[0]) == 0, "Du måste ange ett riktigt textnummer.\n");
	cmd_delete(atoi(argv[0]));
	return 0;
}

static int
exec_other_sync(int argc, char *argv[])
{
	TT(argc > 0, "Synkronisera tar inga argument.\n");
	rk_sync();
	return 0;
}

static int
exec_other_create_person(int argc, char *argv[])
{
	TT(argc > 0, "Skapa person tar inga argument.\n");
	cmd_create_person();
	return 0;
}

static int
exec_conf_delete(int argc, char *argv[])
{
	LF;
	TT(argc == 0, "Du måste ange namnet.\n");
	cmd_erase(re_concat(argc, argv));
	return 0;
}

static int
exec_conf_create(int argc, char *argv[])
{
	LF;
	TT(argc > 0, "Skapa möte tar inga argument.\n");
	cmd_create();
	return 0;
}

static int
exec_text_copy(int argc, char *argv[])
{
	LF;
	TT(argc > 0, "Kopia tar inga argument.\n");
	cmd_copy();
	return 0;
}

static int
exec_other_exec(int argc, char *argv[])
{
	if (argc == 0)
		system("/bin/sh");
	else
		system(re_concat(argc, argv));
	return 0;
}

static int
exec_info_set_motd(int argc, char *argv[])
{
	LF;
	NWA;
	TT(argc == 0, "Du måste ange vem du vill sätta lapp för.\n");
	write_set_motd(re_concat(argc, argv));
	return 0;
}

static int
exec_info_remove_motd(int argc, char *argv[])
{
	LF;
	NWA;
	TT(argc == 0, "Du måste ange vem du vill sätta lapp för.\n");
	write_remove_motd(re_concat(argc, argv));
	return 0;
}

static int
exec_text_move(int argc, char *argv[])
{
	LF;
	NWA;
	TT(argc != 0, "Flytta tar inga argument.\n");
	cmd_move_text();
	return 0;
}

