/* $Id: parse_eng.h,v 1.2 2000/11/22 21:26:34 jens Exp $ */
#ifndef PARSE_ENG_H
#define PARSE_ENG_H

typedef int (*cmd_func_t)(int argc, char *argv[]);

typedef struct cl_ce_que cmd_lst_t;

#define PE_NO_ARG	0
#define	PE_NUM_ARG	1
#define	PE_STR_ARG	2


cmd_lst_t *parse_new_cmd_lst(void);
void parse_free_cmd_lst(cmd_lst_t *);
void parse_add_cmd(cmd_lst_t *,
	const char *name, int prio, int takes_arg, cmd_func_t exec);
void parse_add_alias(cmd_lst_t *, const char *alias, int argc, char *argv[]);
void parse_del_alias(cmd_lst_t *, const char *alias);
void parse_list_alias(cmd_lst_t *);

int parse_exec(cmd_lst_t *, const char *);


#endif /*PARSE_ENG_H*/

