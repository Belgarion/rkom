/* $Id: parse_eng.c,v 1.13 2006/10/20 16:55:58 offe Exp $ */

#include <sys/param.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <err.h>

#include "rkomsupport.h"
#include "container.h"

#include "parse_eng.h"
#include "rkom.h"
#include "rtype.h"

#if !defined(MIN)
#define	MIN(a,b)	(((a)<(b))?(a):(b))
#endif

#include <assert.h>

#define	MAX_LINE 1024

typedef struct cmd_elm cmd_elm_t;
struct cmd_elm {
	int		ce_argc;
	char	**ce_argv;
	int		ce_prio;
	int		ce_takes_arg;
	cmd_func_t ce_exec;
};

static void ce_free(cmd_elm_t *);

/* declare type safe functions */
typedef struct cl_ce_que cmd_lst_t;
CL_TYPE(ce, cmd_elm_t)

/*
 * Done in parse_eng.h
 * typedef struct parse_commands cmds_t;
 */
struct parse_commands {
	cmd_lst_t	*pc_cmds;
	cmd_lst_t	*pc_alias;
};

static void
unescape(const char *str, int *idx, char *next_char,
	int *is_word_sep, int *inside_quotes)
{
	*is_word_sep = 0;
	*next_char = str[*idx];

	if (*next_char == '"') {
		(*idx)++;
		*next_char =str[*idx];

		/* Double quotes counts as one literal quote charachter. */
		if (*next_char == '"')
			return;

		*inside_quotes = !*inside_quotes;
	}

	if (*next_char == ' ') {
		/* Spaces between quotes doesn't count as word terminators. */
		if (*inside_quotes)
			return;

		*is_word_sep = 1;
	}
}

static void
build_argc_argv(const char *str, int *argc, char ***argv)
{
	char	**p, next_char;
	int		i, num_words, num_chars, word_num, char_num;
	int		between_words, is_word_sep, inside_quotes;
	size_t	len;

	len = strlen(str);

	between_words = 1;
	inside_quotes = 0;
	num_chars = num_words = 0;
	for (i = 0; i < len; i++) {
		unescape(str, &i, &next_char, &is_word_sep, &inside_quotes);
		if (is_word_sep) {
			between_words = 1;
			continue;
		}

		num_chars++;
		if (between_words) {
			between_words = 0;
			num_words++;
		}
	}

	*argc = num_words;

	if (num_words == 0)
		return;

	if ((p = malloc(num_words * (sizeof(char *) + 1) + num_chars)) == NULL)
		err(1, "malloc");

	between_words = 1;
	inside_quotes = 0;
	char_num = word_num = 0;
	p[0] = (char *)&p[num_words];
	for (i = 0; i < len; i++) {
		unescape(str, &i, &next_char, &is_word_sep, &inside_quotes);

		if (is_word_sep) {
			/*
			 * char_num != 0 implies that there is something in the last
			 * string and it has to be terminated.
			 */
			if (char_num != 0) {
				/* NULL terminate the last string */
				p[word_num][char_num] = '\0';

				/* XXX */
				if ((word_num + 1) == num_words)
					break;

				/* Point to the new string */
				p[word_num + 1] = &p[word_num][char_num + 1];

				/* Set up the "pointers" for the new string */
				word_num++;
				char_num = 0;
			}
			continue;
		}

		p[word_num][char_num] = next_char;
		char_num++;
	}
	/* NULL terminate the last string */
	p[word_num][char_num] = '\0';
	
	*argv = p;
}

/*
 * Concatenates two tuples of (argc,argv) and puts the result in a new
 * (argc,argv) tuple.
 */
static int
argv_concat(int *new_argc, char ***new_argv,
		int argc1, char *argv1[], int argc2, char *argv2[])
{
	int		i, n_argc;
	size_t	len;
	char	*p, **n_argv;

	len = 0;
	for (i = 0; i < argc1; i++)
		len += strlen(argv1[i]) + 1;
	for (i = 0; i < argc2; i++)
		len += strlen(argv2[i]) + 1;

	n_argc = argc1 + argc2;
	
	if ((n_argv = malloc(sizeof(char *) * (n_argc)  + len)) == NULL)
		return -1;
	*new_argv = n_argv;
	*new_argc = n_argc;

	p = (char *)&n_argv[n_argc];
	for (i = 0; i < argc1; i++) {
		n_argv[i] = p;
		strcpy(p, argv1[i]);
		p += strlen(p) + 1;
	}
	for (i = 0; i < argc2; i++) {
		n_argv[i + argc1] = p;
		strcpy(p, argv2[i]);
		p += strlen(p) + 1;
	}

	return 0;
}

static char **
argv_dup(int argc, char *argv[])
{
	char	**nargv, *p;
	size_t	len;
	int		i;

	len = 0;
	for (i = 0; i < argc; i++)
		len += strlen(argv[i]) + 1;

	if ((nargv = malloc(len + argc * sizeof(char *))) == NULL)
		return NULL;

	p = (char *)&nargv[argc];
	for (i = 0; i < argc; i++) {
		nargv[i] = p;
		strcpy(p, argv[i]);
		p += strlen(p) + 1;
	}
	return nargv;
}

static void
argv_print(int argc, char *argv[])
{
	int		i;

	for (i = 0; i < argc; i++) {
		if (i != 0)
			rprintf(" ");
		rprintf("%s", argv[i]);
	}
}

static void
ce_free(cmd_elm_t *ce)
{
	free(ce->ce_argv);
	free(ce);
}

cmds_t *
parse_new_cmds(void)
{
	cmds_t	*c;

	if ((c = calloc(1, sizeof(*c))) == NULL)
		return NULL;
	if ((c->pc_cmds = cl_ce_init()) == NULL)
		goto ret_bad;
	if ((c->pc_alias = cl_ce_init()) == NULL)
		goto ret_bad;
	return c;

ret_bad:
	if (c->pc_cmds != NULL)
		cl_ce_free(c->pc_cmds, ce_free);
	free(c);
	return NULL;
}


void
parse_free_cmds(cmds_t *c)
{
	cl_ce_free(c->pc_cmds, ce_free);
	cl_ce_free(c->pc_alias, ce_free);
}

void
parse_add_cmd(cmds_t *c,
	const char *str, int prio, int takes_arg, cmd_func_t exec)
{
	cmd_elm_t	*ce;

	if ((ce = calloc(1, sizeof(*ce))) == NULL)
		err(1, "calloc");

	build_argc_argv(str, &ce->ce_argc, &ce->ce_argv);
	ce->ce_prio = prio;
	ce->ce_exec = exec;
	ce->ce_takes_arg = takes_arg;
	if (cl_ce_push(c->pc_cmds, ce) < 0)
		exit(1);
}

void
parse_add_alias(cmds_t *c, int argc, char *argv[])
{
	cmd_elm_t	*ce;
	char		**nargv;
	

	if ((ce = calloc(1, sizeof(*ce))) == NULL)
		err(1, "calloc");

	if ((nargv = argv_dup(argc, argv)) == NULL)
		err(1, "argv_dup");

	ce->ce_argc = argc;
	ce->ce_argv = nargv;
	if (cl_ce_push(c->pc_alias, ce) < 0)
		exit(1);
}

void
parse_del_alias(cmds_t *c, const char *alias)
{
	cmd_elm_t	*ce;
	cmd_lst_t	*cl_pos;
	int			num_deleted;

	num_deleted = 0;
	/* Loop over all aliases */
	for (cl_pos = NULL; cl_ce_walk(c->pc_alias, &cl_pos, &ce) == 0;) {

		if (strcmp(ce->ce_argv[0], alias) != 0)
			continue;

		num_deleted++;
		rprintf("Tar bort aliaset '%s' -> '", ce->ce_argv[0]);
		argv_print(ce->ce_argc - 1, ce->ce_argv + 1);
		rprintf("'\n");
		if (cl_ce_rem_pos(cl_pos) < 0) {
			warn("cl_rem_pos");
			continue;
		}

		ce_free(ce);
	}

	if (num_deleted == 0)
		rprintf("'%s' matchar inget befintligt alias\n", alias);
}

void
parse_list_alias(cmds_t *c)
{
	cmd_elm_t	*ce;
	cmd_lst_t	*cl_pos;
	int			alias_num;

	alias_num = 0;
	/* Loop over all aliases */
	for (cl_pos = NULL; cl_ce_walk(c->pc_alias, &cl_pos, &ce) == 0;) {

		if (alias_num == 0)
			rprintf("Du har följande alias definierade:\n");

		alias_num++;
		rprintf("'%s' -> '", ce->ce_argv[0]);
		argv_print(ce->ce_argc - 1, ce->ce_argv + 1);
		rprintf("'\n");
	}

	if (alias_num == 0)
		rprintf("Du har inga alias definierade\n");
}

int
parse_exec(cmds_t *c, const char *str)
{
	cmd_lst_t	*cl_pos, *cl_matches;
	cmd_elm_t	*al_ce, *ce, *ce_max_prio, *ce_arg_match;
	int			nargc, argc, i, j, min_argc, max_prio, ret, arg_match, isalias;
	char		**nargv, **argv;

	ret = 0;
	isalias = 0;
	nargc = 0;
	nargv = NULL;
	al_ce = NULL;

	build_argc_argv(str, &argc, &argv);

	/* Check for an alias and substitute if found */
	for (cl_pos = NULL; cl_ce_walk(c->pc_alias, &cl_pos, &ce) == 0;) {
		/* Only exactly matching aliases are substiting the command */
		if (strcmp(ce->ce_argv[0], argv[0]) != 0)
			continue;

		/* An alias was found, substitute argv and break the loop */
		if (argv_concat(&nargc, &nargv,
			ce->ce_argc - 1, ce->ce_argv + 1, argc - 1, argv + 1) < 0)
			err(1, "argv_concat");
		free(argv);
		argv = nargv;
		argc = nargc;
		isalias = 1;
		al_ce = ce;
		break;
	}

	if ((cl_matches = cl_ce_init()) == NULL)
		exit(1);

	max_prio = arg_match = -1;
	ce_max_prio = ce_arg_match = NULL;

	/* First, build a list of possible matching commands */
	for (cl_pos = NULL; cl_ce_walk(c->pc_cmds, &cl_pos, &ce) == 0;) {

		/*
		 * Exceeding argv's may be arguments for the command, let's
		 * analyze the argument later.
		 */
		min_argc = MIN(argc, ce->ce_argc);

		/* Check the actual strings of the command */
		for (i = 0; i < min_argc; i++) {
			for (j = 0;
				argv[i][j] == ce->ce_argv[i][j] && argv[i][j] != '\0';
				j++);

			/* argv[i] cannot match if it's longer than ce->ce_argv[i] */
			if (argv[i][j] != '\0')
				goto skip;
		}

		/* Let's have a look at the argument for the command. */
		if (argc > ce->ce_argc) {
			switch(ce->ce_takes_arg) {
			case PE_NO_ARG:
				/* Don't accept any arguments */
				goto skip;
			case PE_NUM_ARG:
				/* Only accept arguemnts starting with a number */
				if (!isdigit((int)argv[ce->ce_argc][0]))
					goto skip;
				break;
			case PE_STR_ARG:
				/* Accept any argument */
				break;
			default:
				assert(0);
			}
		}

		if (cl_ce_push(cl_matches, ce) < 0)
			exit(1);

		/*
		 * Give a command with an exact match on number of argc's sligthly
		 * higher prio.
		 */
		if (ce->ce_argc == argc) {
			if (arg_match < 0) {
				arg_match++;
				ce_arg_match = ce;
			} else
				ce_arg_match = NULL;
		}

		/*
		 * If two command has the same prio as max_prio they will
		 * null out each other. So you should avoid it by choosing
		 * which command should have the higher prio.
		 */
		if (ce->ce_prio > max_prio) {
			max_prio = ce->ce_prio;
			ce_max_prio = ce;
		} else if (ce->ce_prio == max_prio) {
			ce_max_prio = NULL;
			if (max_prio != 0) {
				warnx("Argh! Two commands has the same prio.");
				warnx("Read comments in parse.c and parse_eng.c:parse_exec()");
				warnx("and ask Jens if you don't understand.");
			}
		}

	/* An escape label for a nested for-statement */
skip:
		;
	}

	if (max_prio < 0) {
		/* There are no matches */
		ret = -1;
		rprintf("Okänt kommando: '%s'\n", str);
		if (isalias) {
			rprintf("Du bör se över ditt alias: '%s' -> '", al_ce->ce_argv[0]);
			argv_print(al_ce->ce_argc - 1, al_ce->ce_argv + 1);
			rprintf("'\n");
		}

	} else if (ce_max_prio != NULL) {
		/* We have only one possible match */
		ce = ce_max_prio;
		ret = ce->ce_exec(argc - ce->ce_argc, argv + ce->ce_argc);

	} else {
		if (ce_arg_match != NULL) {
			ce = ce_arg_match;
			ret = ce->ce_exec(argc - ce->ce_argc, argv + ce->ce_argc);
		} else {
			/* There are several matches */
			ret = -1;
			rprintf("Flertydigt kommando. Du kan mena:\n\n");
			for (cl_pos = NULL; cl_ce_walk(cl_matches, &cl_pos, &ce) == 0;) {
				for (i = 0; i < ce->ce_argc; i++)
					rprintf("%s ", ce->ce_argv[i]);
				rprintf("\n");
			}
			if (isalias) {
				rprintf("Du bör se över ditt alias: '%s' -> '",
					al_ce->ce_argv[0]);
				argv_print(al_ce->ce_argc - 1, al_ce->ce_argv + 1);
				rprintf("'\n");
			}
		}
	}

	if (cl_matches != NULL) {
		cl_ce_free(cl_matches, NULL);
		cl_matches = NULL;
	}
	free(argv);
	return ret;
}
