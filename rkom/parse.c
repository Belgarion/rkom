/* $Id: parse.c,v 1.2 2000/11/18 10:36:04 ragge Exp $ */

#include <sys/param.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

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


#include "parse_eng.h"
#include "parse.h"

#if defined(__STDC__) || defined(__cplusplus)
#	define CC_2(a,b)        a ## b
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

/* Commands for conferences */
DCMD(conf_where);
DCMD(conf_goto);
DCMD(conf_goto_next);
DCMD(conf_list);
DCMD(conf_leave);

/* Commands for texts */
DCMD(text_put);
DCMD(text_add_rcpt);
DCMD(text_add_cmt_to);
DCMD(text_forget);
DCMD(text_show);
DCMD(text_edit);
DCMD(text_mark);
DCMD(text_unmark);
DCMD(text_list_marked);
DCMD(text_save);

/* Commands for online communication */
DCMD(com_who);
DCMD(com_who_visible);
DCMD(com_who_invisible);
DCMD(com_who_clients);
DCMD(com_send);
DCMD(com_say);

/* Commands for information */
DCMD(info_list_commands);
DCMD(info_flags);
DCMD(info_time);
DCMD(info_saveflags);

/* Other commands */
DCMD(other_set);
DCMD(other_login);
DCMD(other_logout);
DCMD(other_quit);

struct command_list {
	char	*cl_str;
	int		cl_prio;
	int		cl_takes_arg;
	cmd_func_t cl_exec;
};

#define DROW(name, prio, takes_arg, command) \
{ name, prio, takes_arg, CC_2(exec_,command) },

struct command_list commands[] = {
/* Commands for reading */
DROW("n�sta inl�gg",			0,PE_NO_ARG,read_next_text)
DROW("n�sta kommentar",			0,PE_NO_ARG,read_next_cmt)
DROW("�terse",				0,PE_NUM_ARG,read_see_again_cmt_no)
DROW("�terse kommenterade",		1,PE_NO_ARG,read_see_again_cmt)
DROW("lista nyheter",			0,PE_NO_ARG,read_list_news)
DROW("endast",					0,PE_NUM_ARG,read_only)
DROW("igen",					0,PE_NO_ARG,read_again)
DROW("hoppa",					0,PE_NO_ARG,read_jump)

/* Commands for writing */
DROW("inl�gg",					0,PE_NO_ARG,write_new)
DROW("kommentera",				0,PE_NUM_ARG,write_cmt_no)
DROW("kommentera",				1,PE_NO_ARG,write_cmt)
DROW("kommentera f�reg�nde",			0,PE_NO_ARG,write_cmt_last)
DROW("fotnot",					0,PE_NUM_ARG,write_footnote_no)
DROW("fotnot",					1,PE_NO_ARG,write_footnote)
DROW("brev",					0,PE_STR_ARG,write_letter)

/* Commands for conferences */
DROW("var",						0,PE_NO_ARG,conf_where)
DROW("g�",						0,PE_STR_ARG,conf_goto)
DROW("n�sta m�te",				0,PE_NO_ARG,conf_goto_next)
DROW("lista m�ten",				0,PE_NO_ARG,conf_list)
DROW("uttr�da",					0,PE_STR_ARG,conf_leave)

/* Commands for texts */
DROW("l�gga",					0,PE_NO_ARG,text_put)
DROW("mottagare:",				0,PE_STR_ARG,text_add_rcpt)
DROW("kommentar till:",			0,PE_NUM_ARG,text_add_cmt_to)
DROW("gl�m",					0,PE_NO_ARG,text_forget)
DROW("hela",					0,PE_NO_ARG,text_show)
DROW("redigera",				0,PE_NO_ARG,text_edit)
DROW("markera",					0,PE_NUM_ARG,text_mark)
DROW("avmarkera",				0,PE_NUM_ARG,text_unmark)
DROW("lista markerade",			0,PE_NO_ARG,text_list_marked)
DROW("spara",					0,PE_STR_ARG,text_save)
DROW("spara flaggor",				0,PE_NO_ARG,info_saveflags)

/* Commands for online communication */
DROW("vilka",					0,PE_NO_ARG,com_who)
DROW("vilka synliga",			0,PE_NO_ARG,com_who_visible)
DROW("vilka osynliga",			0,PE_NO_ARG,com_who_invisible)
DROW("vilka klienter",			0,PE_NO_ARG,com_who_clients)
DROW("s�nd",					0,PE_NO_ARG,com_send)
DROW("s�g",						0,PE_STR_ARG,com_say)

/* Commands for information */
DROW("lista kommandon",			0,PE_NO_ARG,info_list_commands)
DROW("flaggor",					0,PE_NO_ARG,info_flags)
DROW("tiden",					0,PE_NO_ARG,info_time)
DROW("hj�lp",					0,PE_NO_ARG,info_list_commands)

/* Other commands */
DROW("s�tt",					0,PE_STR_ARG,other_set)
DROW("login",					0,PE_STR_ARG,other_login)
DROW("logout",					0,PE_NO_ARG,other_logout)
DROW("sluta",					0,PE_NO_ARG,other_quit)

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

static cmd_lst_t *cmds;
static int inited = 0;

static void p_init(void);

static void
p_init(void)
{
	struct command_list *c;

	c = commands;
	cmds = parse_new_cmd_lst();
	for (c = commands; c->cl_str != NULL; c++)
		parse_add_cmd(cmds, c->cl_str, c->cl_prio, c->cl_takes_arg, c->cl_exec);
}

int
exec_cmd(const char *str)
{
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
			printf("Ok�nd prompt %s\n", prompt);
		return 0;
	}

	return parse_exec(cmds, str);
}

/* Helpers */
#define	XXTEST(name, test, text) \
static int name(void) {if(test)printf(text);else return 0;return 1;}
#define TT(cmp, str) if (cmp) {printf(str);return 0;}

XXTEST(nwa, is_writing, "Du h�ller redan p� att skriva en text.\n")
#define	NWA if (nwa()) return 0
XXTEST(owa, is_writing == 0, "Du skriver ingen text just nu.\n")
#define OWA if (owa()) return 0
XXTEST(lf, myuid == 0, "Du m�ste logga in f�rst.\n")
#define LF if (lf()) return 0
XXTEST(mhc, curconf == 0, "Du m�ste g� till ett m�te f�rst.\n")
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
	printf("%s: ", __FUNCTION__); \
	for (i = 0; i < argc; i++) \
		printf("%s ", argv[i]); \
	printf("\n"); \
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
	next_resee_text(atoi(argv[0]));
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
	TT(argc < 1, "Du m�ste ange hur m�nga du endast vill se.\n");
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
	TT(argc < 1, "Du m�ste ange mottagare.\n");
	write_brev(re_concat(argc, argv));
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
	TT(argc == 0, "Handhavande:\ng� <m�tesnamn>\n");
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
	TT(argc != 1, "Handhavande:\nuttr�da <m�tesnamn>\n");
	cmd_leave(argv[0]);
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
exec_text_add_rcpt(int argc, char *argv[])
{
	OWA;
	TT(argc != 1, "Handhavande:\nmottagare: <mottagarnamn>\n");
	write_rcpt(argv[0]);
	return 0;
}

static int
exec_text_add_cmt_to(int argc, char *argv[])
{
	OWA;
	TT(argc != 1, "Handhavande:\nkommentar till: <�rendenummer>\n");
	write_comment(argv[0]);
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
	TT(argc != 1, "Handhavande:\nmarkera <�rendenummer>\n");
	list_mark(argv[0]);
	return 0;
}

static int
exec_text_unmark(int argc, char *argv[])
{
	LF;
	TT(argc != 1, "Handhavande:\navmarkera <�rendenummer>\n");
	list_unmark(argv[0]);
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
	cmd_vilka(NULL);
	return 0;
}

static int
exec_com_who_visible(int argc, char *argv[])
{
	cmd_vilka("synliga");
	return 0;
}

static int
exec_com_who_invisible(int argc, char *argv[])
{
	cmd_vilka("osynliga");
	return 0;
}

static int
exec_com_who_clients(int argc, char *argv[])
{
	cmd_vilka("klienter");
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
	TT(argc != 1, "Handhavande:\ns�g <mottagare>\n");
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
//	LF;
	set_saveflags();
	return 0;
}

static int
exec_info_flags(int argc, char *argv[])
{
//	LF;
	set_flags();
	return 0;
}

static int
exec_info_time(int argc, char *argv[])
{
	cmd_tiden(NULL);
	return 0;
}


/* Other commands */
static int
exec_other_set(int argc, char *argv[])
{
//	LF;
	TT(argc != 2, "Handhavande:\ns�tt <flaggnamn> <v�rde>\n");
	set_setflag(argv[0], argv[1]);
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

