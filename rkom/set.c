

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <err.h>

#include "rkom_proto.h"
#include "rkom.h"

#include "set.h"


char *user, *pass, *server;
int no_user_active;

#define	STRING	1
#define	INT	2

struct setcmnds {
	char *name;
	int type;
	void *var;
} setcmnds[] = {
	{"user", STRING, &user },
	{"pass", STRING, &pass },
	{"server", STRING, &server },
};

static int ncs = sizeof(setcmnds) / sizeof(setcmnds[0]);

void
parsefile(char *fname)
{
	FILE *fd;
	int line = 0, i;
	char buf[80], *arg;

	if (fname == NULL) {
		char *home = getenv("HOME");
		if (home) {
			fname = alloca(strlen(home) + 9);
			strcpy(fname, home);
			strcat(fname, "/");
		} else {
			fname = alloca(8);
			*fname = 0;
		}
		strcat(fname, ".rkomrc");
	}

	if ((fd = fopen(fname, "r")) == NULL)
		return;
	while (fgets(buf, sizeof(buf), fd) != NULL) {
		line++;
		if (buf[0] == '#')
			continue;
		if ((arg = index(buf, '#'))) {
			*arg-- = 0;
			while (isspace(*arg) && arg != &buf[0])
				*arg-- = 0;
		}
		if (buf[0] == 0)
			continue;
		arg = index(buf, '=');
		if (arg)
			*arg++ = 0;

		for (i = 0; i < ncs; i++) {
			if (strcmp(buf, setcmnds[i].name))
				continue;

			if (setcmnds[i].type == STRING) {
				if (arg == 0) {
					rprintf("%s: syntax error at line %d\n",
					    fname, line);
					break;
				}
				*(char **)setcmnds[i].var = strdup(arg);
			} else { /* == INT */
				*(int *)setcmnds[i].var = 1;
			}
			break;
		}
		if (i == ncs)
			rprintf("%s: bad line %d\n", fname, line);
	}
	fclose(fd);
}

static struct rk_val commonvars[] = {
	{ "created-texts-are-read", "1" },
	{ "dashed-lines", "1" },
	{ "presence-messages", "1" },
	{ "print-number-of-unread-on-entrance", "1" },
	{ "read-depth-first", "1" },
	{ "reading-puts-comments-in-pointers-last", "1" },
	{ "confirm-multiple-recipients", "0" },
	{ "default-mark", "100" },
};
static int ncommonvars = sizeof(commonvars)/sizeof(commonvars[0]);
static int cmod, rmod;

static struct rk_val rkomvars[] = {
	{ "use-editor", "0" },
	{ "show-writer-after-text", "1" },
	{ "idle-hide-in-who-list", "30" },
	{ "kom-mercial", "Kör raggeklienten!" },
	{ "unread-long-format", "1" },
	{ "short-time-format", "0" },
	{ "ignore-ctrl-c", "0" },
};
static int nrkomvars = sizeof(rkomvars)/sizeof(rkomvars[0]);

static void
put_in_vars(struct rk_uarea *ru, struct rk_val *w, int len)
{
	int i, j;

	for (i = 0; i < ru->ru_val.ru_val_len; i++) {
		for (j = 0; j < len; j++) {
			if (strcmp(ru->ru_val.ru_val_val[i].rv_var, 
			    w[j].rv_var) == 0) {
				w[j].rv_val =
				    strdup(ru->ru_val.ru_val_val[i].rv_val);
				break;
			}
		}
	}
}

void
readvars()
{
	struct rk_uarea *ru;

	ru = rk_get_uarea("common");
	if (ru->ru_retval == 0)
		put_in_vars(ru, commonvars, ncommonvars);
	free(ru);

	ru = rk_get_uarea("rkom");
	if (ru->ru_retval == 0)
		put_in_vars(ru, rkomvars, nrkomvars);
	free(ru);
}

int
iseql(char *var, char *val)
{
	int i;

	for (i = 0; i < ncommonvars; i++)
		if ((strcmp(commonvars[i].rv_var, var) == 0) &&
		    (strcmp(commonvars[i].rv_val, val) == 0))
			return 1;
	for (i = 0; i < nrkomvars; i++)
		if ((strcmp(rkomvars[i].rv_var, var) == 0) &&
		    (strcmp(rkomvars[i].rv_val, val) == 0))
			return 1;
	return 0;
}

int
isneq(char *var, char *val)
{
	int i;

	for (i = 0; i < ncommonvars; i++)
		if ((strcmp(commonvars[i].rv_var, var) == 0) &&
		    (strcmp(commonvars[i].rv_val, val) == 0))
			return 0;
	for (i = 0; i < nrkomvars; i++)
		if ((strcmp(rkomvars[i].rv_var, var) == 0) &&
		    (strcmp(rkomvars[i].rv_val, val) == 0))
			return 0;
	return 1;
}

char *
getval(char *var)
{
	int i;

	for (i = 0; i < ncommonvars; i++)
		if (strcmp(commonvars[i].rv_var, var) == 0)
			return commonvars[i].rv_val;
	for (i = 0; i < nrkomvars; i++)
		if (strcmp(rkomvars[i].rv_var, var) == 0)
			return rkomvars[i].rv_val;
	return 0;
}

void
set_flags()
{
	int i;

	rprintf("(Lista) flaggor\n");
	for (i = 0; i < ncommonvars; i++)
		rprintf("\t%s  %s\n", commonvars[i].rv_var, commonvars[i].rv_val);
	for (i = 0; i < nrkomvars; i++)
		rprintf("\t%s  %s\n", rkomvars[i].rv_var, rkomvars[i].rv_val);
	rprintf("\n");
}

void
set_setflag(char *name, char *val)    
{
	int inc, match, i, nc;

	match = nc = 0;
	for (i = 0; i < ncommonvars; i++) {
		if (bcmp(commonvars[i].rv_var, name, strlen(name)) == 0) {
			match++;
			nc = i;
		}
	}
	inc = match;
	for (i = 0; i < nrkomvars; i++) {
		if (bcmp(rkomvars[i].rv_var, name, strlen(name)) == 0) {
			match++;
			nc = i;
		}
	}
	if (match == 0) {
		rprintf("Flaggan finns inte.\n");
		return;
	} else if (match > 1) {
		rprintf("Flaggans namn är inte entydigt.\n");
		return;
	}
	if (inc) {
		commonvars[nc].rv_val = strdup(val);
		cmod++;
	} else {
		rkomvars[nc].rv_val = strdup(val);
		rmod++;
	}
/* HACK! some variables need interaction with the server when changed */
	if (bcmp(name, "kom-mercial", strlen(name)) == 0)
		if (rk_whatido(val))
			rprintf("Byta kom-mercial sket sej.\n");
	if (bcmp(name, "ignore-ctrl-c", strlen(name)) == 0) {
		if (strcmp(val, "0") == 0)
			signal(SIGINT, SIG_DFL);
		else
			signal(SIGINT, SIG_IGN);
	}
/* END HACK */
}

void
set_saveflags()
{
	struct rk_uarea ru;

	if (cmod) {
		ru.ru_val.ru_val_len = ncommonvars;
		ru.ru_val.ru_val_val = commonvars;
		if (rk_set_uarea("common", &ru))
			rprintf("rk_set_uarea(common) sket sej\n");
	}
	if (rmod) {
		ru.ru_val.ru_val_len = nrkomvars;
		ru.ru_val.ru_val_val = rkomvars;
		if (rk_set_uarea("rkom", &ru))
			rprintf("rk_set_uarea(rkom) sket sej\n");
	}
	cmod = rmod = 0;
}
