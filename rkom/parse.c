/* $Id: parse.c,v 1.1 2000/11/05 16:26:19 jens Exp $ */

#include <sys/param.h>

#include <stdio.h>
#include <string.h>

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
DCMD(read_list_news);
DCMD(read_only);
DCMD(read_again);
DCMD(read_jump);

/* Commands for writing */
DCMD(write_new);
DCMD(write_cmt);
DCMD(write_cmt_last);
DCMD(write_footnote);
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
DROW("nästa inlägg",			0,PE_NO_ARG,read_next_text)
DROW("nästa kommentar",			0,PE_NO_ARG,read_next_cmt)
DROW("återse kommenterade",		0,PE_NO_ARG,read_see_again_cmt)
DROW("lista nyheter",			0,PE_NO_ARG,read_list_news)
DROW("endast",					0,PE_NUM_ARG,read_only)
DROW("igen",					0,PE_NO_ARG,read_again)
DROW("hoppa",					0,PE_NO_ARG,read_jump)

/* Commands for writing */
DROW("inlägg",					0,PE_NO_ARG,write_new)
DROW("kommentera",				1,PE_NO_ARG,write_cmt)
DROW("kommentera föregånde",	0,PE_NO_ARG,write_cmt_last)
DROW("fotnot",					0,PE_NO_ARG,write_footnote)
DROW("brev",					0,PE_STR_ARG,write_letter)

/* Commands for conferences */
DROW("var",						0,PE_NO_ARG,conf_where)
DROW("gå",						0,PE_STR_ARG,conf_goto)
DROW("nästa möte",				0,PE_NO_ARG,conf_goto_next)
DROW("lista möten",				0,PE_NO_ARG,conf_list)
DROW("utträda",					0,PE_STR_ARG,conf_leave)

/* Commands for texts */
DROW("lägga",					0,PE_NO_ARG,text_put)
DROW("mottagare:",				0,PE_STR_ARG,text_add_rcpt)
DROW("kommentar till:",			0,PE_NUM_ARG,text_add_cmt_to)
DROW("glöm",					0,PE_NO_ARG,text_forget)
DROW("hela",					0,PE_NO_ARG,text_show)
DROW("redigera",				0,PE_NO_ARG,text_edit)
DROW("markera",					0,PE_NUM_ARG,text_mark)
DROW("avmarkera",				0,PE_NUM_ARG,text_unmark)
DROW("lista markerade",			0,PE_NO_ARG,text_list_marked)
DROW("spara",					0,PE_STR_ARG,text_save)

/* Commands for online communication */
DROW("vilka",					0,PE_NO_ARG,com_who)
DROW("vilka synliga",			0,PE_NO_ARG,com_who_visible)
DROW("vilka osynliga",			0,PE_NO_ARG,com_who_invisible)
DROW("vilka klienter",			0,PE_NO_ARG,com_who_clients)
DROW("sänd",					0,PE_NO_ARG,com_send)
DROW("säg",						0,PE_STR_ARG,com_say)

/* Commands for information */
DROW("lista kommandon",			0,PE_NO_ARG,info_list_commands)
DROW("flaggor",					0,PE_NO_ARG,info_flags)
DROW("tiden",					0,PE_NO_ARG,info_time)

/* Other commands */
DROW("sätt",					0,PE_STR_ARG,other_set)
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
		/* Replace cmd_tiden with the default action at this moment */
		printf("default action\n");
		cmd_tiden(NULL);
		return 0;
	}

	return parse_exec(cmds, str);
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
	next_text(NULL);
	return 0;
}

static int
exec_read_next_cmt(int argc, char *argv[])
{
	next_comment(NULL);
	return 0;
}

static int
exec_read_see_again_cmt(int argc, char *argv[])
{
	if (argc < 1) {
		printf("Du måste de ett arguemnt till \"återse\"\n");
		return -1;
	}
	if (argc > 1) {
		printf("\"återse\" tar endast ett argument\n");
		return -1;
	}
	next_resee(argv[0]);
	return 0;
}

static int
exec_read_list_news(int argc, char *argv[])
{
	P_CALL_INFO;

	return 0;
}

static int
exec_read_only(int argc, char *argv[])
{
	P_CALL_INFO;

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
	write_cmnt(NULL);
	return 0;
}

static int
exec_write_cmt_last(int argc, char *argv[])
{
	P_CALL_INFO;

	return 0;
}

static int
exec_write_footnote(int argc, char *argv[])
{
	write_footnote(NULL);
	return 0;
}

static int
exec_write_letter(int argc, char *argv[])
{
	if (argc < 1) {
		printf("Du måste ange mottagare.\n");
		return -1;
	}
	if (argc > 1) {
		printf("\"brev\" tar endast ett argument\n");
		return -1;
	}
	write_brev(argv[0]);
	return 0;
}


/* Commands for conferences */
static int
exec_conf_where(int argc, char *argv[])
{
	cmd_where(NULL);
	return 0;
}

static int
exec_conf_goto(int argc, char *argv[])
{
	if (argc != 1) {
		printf("Handhavande:\ngå <mötesnamn>\n");
		return -1;
	}
	cmd_goto(argv[0]);
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
	if (argc != 1) {
		printf("Handhavande:\nutträda <mötesnamn>\n");
		return -1;
	}
	cmd_leave(argv[0]);
	return 0;
}


/* Commands for texts */
static int
exec_text_put(int argc, char *argv[])
{
	write_put(NULL);
	return 0;
}

static int
exec_text_add_rcpt(int argc, char *argv[])
{
	if (argc != 1) {
		printf("Handhavande:\nmottagare: <mottagarnamn>\n");
		return -1;
	}
	write_rcpt(argv[0]);
	return 0;
}

static int
exec_text_add_cmt_to(int argc, char *argv[])
{
	if (argc != 1) {
		printf("Handhavande:\nkommentar till: <ärendenummer>\n");
		return -1;
	}
	write_comment(argv[0]);
	return 0;
}

static int
exec_text_forget(int argc, char *argv[])
{
	write_forget(NULL);
	return 0;
}

static int
exec_text_show(int argc, char *argv[])
{
	write_whole(NULL);
	return 0;
}

static int
exec_text_edit(int argc, char *argv[])
{
	write_editor(NULL);
	return 0;
}

static int
exec_text_mark(int argc, char *argv[])
{
	if (argc != 1) {
		printf("Handhavande:\nmarkera <ärendenummer>\n");
		return -1;
	}
	list_mark(argv[0]);
	return 0;
}

static int
exec_text_unmark(int argc, char *argv[])
{
	if (argc != 1) {
		printf("Handhavande:\navmarkera <ärendenummer>\n");
		return -1;
	}
	list_unmark(argv[0]);
	return 0;
}

static int
exec_text_list_marked(int argc, char *argv[])
{
	list_marked(NULL);
	return 0;
}

static int
exec_text_save(int argc, char *argv[])
{
	if (argc != 1) {
		printf("Handhavande:\nspara <filnamn>\n");
		return -1;
	}
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
	cmd_send(NULL);
	return 0;
}

static int
exec_com_say(int argc, char *argv[])
{
	if (argc != 1) {
		printf("Handhavande:\nsäg <mottagare>\n");
		return -1;
	}
	cmd_say(argv[0]);
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
exec_info_flags(int argc, char *argv[])
{
	set_flags(NULL);
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
	char	buf[100];

	if (argc != 2) {
		printf("Handhavande:\nsätt <flaggnamn> <värde>\n");
		return -1;
	}
	strlcpy(buf, argv[0], sizeof(buf));
	strlcat(buf, " ", sizeof(buf));
	strlcat(buf, argv[1], sizeof(buf));
	set_setflag(buf);
	return 0;
}

static int
exec_other_login(int argc, char *argv[])
{
	char	buf[100];
	int		i;

	if (argc == 0) {
		printf("Handhavande:\nlogin <namn>\n");
		return -1;
	}
	buf[0] = '\0';
	for (i = 0; i < argc; i++) {
		strlcat(buf, argv[i], sizeof(buf));
		strlcat(buf, " ", sizeof(buf));
	}
	cmd_login(buf);
	return 0;
}

static int
exec_other_logout(int argc, char *argv[])
{
	cmd_logout(NULL);
	return 0;
}

static int
exec_other_quit(int argc, char *argv[])
{
	cmd_sluta(NULL);
	return 0;
}


