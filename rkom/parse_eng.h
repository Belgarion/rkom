/* $Id: parse_eng.h,v 1.3 2000/11/28 22:32:21 jens Exp $ */
#ifndef PARSE_ENG_H
#define PARSE_ENG_H

typedef int (*cmd_func_t)(int argc, char *argv[]);

typedef struct parse_commands cmds_t;

#define PE_NO_ARG	0
#define	PE_NUM_ARG	1
#define	PE_STR_ARG	2


cmds_t *parse_new_cmds(void);
void parse_free_cmds(cmds_t *);
void parse_add_cmd(cmds_t *,
	const char *name, int prio, int takes_arg, cmd_func_t exec);
void parse_add_alias(cmds_t *, int argc, char *argv[]);
void parse_del_alias(cmds_t *, const char *alias);
void parse_list_alias(cmds_t *);

int parse_exec(cmds_t *, const char *);


#endif /*PARSE_ENG_H*/

