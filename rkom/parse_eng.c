/* $Id: parse_eng.c,v 1.2 2000/11/22 11:58:25 ragge Exp $ */

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <err.h>

#include "container.h"

#include "parse_eng.h"
#include "rkom.h"

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

/*
 * Done in parse_eng.h:
 * typedef struct cl_ce_que cmd_lst_t;
 */
/* declare type safe functions */
CL_TYPE(ce, cmd_elm_t)



static void
build_argc_argv(const char *str, int *argc, char ***argv)
{
	char	**p;
	int		i, num_words, num_chars, between_words, word_num, char_num;
	size_t	len;

	len = strlen(str);

	between_words = 1;
	num_chars = num_words = 0;
	for (i = 0; i < len; i++) {
		if (str[i] == ' ') {
			between_words = 1;
			continue;
		}

		/* str[i] != ' ' */
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
	char_num = word_num = 0;
	p[0] = (char *)&p[num_words];
	for (i = 0; i < len; i++) {
		if (str[i] == ' ') {

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

				/* Start the new string */
				p[word_num + 1] = &p[word_num][char_num + 1];

				/* Set up the "pointers" for the new string */
				word_num++;
				char_num = 0;
			}

			continue;
		}

		/* str[i] != ' ' */
		p[word_num][char_num] = str[i];
		char_num++;
	}
	/* NULL terminate the last string */
	p[word_num][char_num] = '\0';
	
	*argv = p;
}

cmd_lst_t *
parse_new_cmd_lst(void)
{
	return cl_ce_init();
}


static void
ce_free(cmd_elm_t *ce)
{
	free(ce->ce_argv);
	free(ce);
}

void
parse_free_cmd_lst(cmd_lst_t *cl)
{
	cl_ce_free(cl, ce_free);
}

void
parse_add_cmd(cmd_lst_t *cl,
	const char *str, int prio, int takes_arg, cmd_func_t exec)
{
	cmd_elm_t	*ce;

	if ((ce = malloc(sizeof(*ce))) == NULL)
		err(1, "malloc");

	build_argc_argv(str, &ce->ce_argc, &ce->ce_argv);
	ce->ce_prio = prio;
	ce->ce_exec = exec;
	ce->ce_takes_arg = takes_arg;
	if (cl_ce_push(cl, ce) < 0)
		exit(1);
}

int
parse_exec(cmd_lst_t *cl, const char *str)
{
	cmd_lst_t	*cl_pos, *cl_matches;
	cmd_elm_t	*ce, *ce_max_prio, *ce_arg_match;
	int			argc, i, j, min_argc, max_prio, ret, arg_match;
	char		**argv;

	ret = 0;

	if ((cl_matches = cl_ce_init()) == NULL)
		exit(1);


	build_argc_argv(str, &argc, &argv);

	max_prio = arg_match = -1;
	ce_max_prio = ce_arg_match = NULL;

	/* First, build a list of possible matching commands */
	for (cl_pos = NULL; cl_ce_walk(cl, &cl_pos, &ce) == 0;) {

		/*
		 * Exceeding argv's may be arguments for the command, let's
		 * analyze the argument later.
		 */
		min_argc = MIN(argc, ce->ce_argc);

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
				if (!isdigit(argv[ce->ce_argc][0]))
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

		if (ce->ce_prio > max_prio) {
			max_prio = ce->ce_prio;
			ce_max_prio = ce;
		} else if (ce->ce_prio == max_prio)
			ce_max_prio = NULL;

	/* An escape label for a nested for-statement */
skip:
	}

	if (max_prio < 0) {
		/* There are no matches */
		ret = -1;
		rprintf("Okänt kommando: '%s'\n", str);

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
		}
	}

	if (cl_matches != NULL) {
		cl_ce_free(cl_matches, NULL);
		cl_matches = NULL;
	}
	return ret;
}

