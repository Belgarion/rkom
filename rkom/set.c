

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>

#include "rkom_proto.h"
#include "exported.h"

#include "set.h"


char *user, *pass, *server;
int use_editor, no_user_active;

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
	{"use_editor", INT, &use_editor },
	{"no_user_active", INT, &no_user_active },
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
					printf("%s: syntax error at line %d\n",
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
			printf("%s: bad line %d\n", fname, line);
	}
	fclose(fd);
}

static struct rk_val *rkv;
static int rkn;

void
readvars()
{
	struct rk_uarea *ru;

	ru = rk_get_uarea("common");
	if (ru->ru_retval || (ru->ru_val.ru_val_len == 0))
		return;

	rkn = ru->ru_val.ru_val_len;
	rkv = calloc(sizeof(*rkv), rkn);
	bcopy(ru->ru_val.ru_val_val, rkv, sizeof(*rkv) * rkn);
	free(ru);
}

int
iseql(char *var, char *val)
{
	int i;

	if (rkv == 0)
		return 0;
	for (i = 0; i < rkn; i++)
		if ((strcmp(rkv->rv_var, var) == 0) &&
		    (strcmp(rkv->rv_val, val) == 0))
			return 1;
	return 0;
}

int
isneq(char *var, char *val)
{
	int i;

	if (rkv == 0)
		return 1;
	for (i = 0; i < rkn; i++)
		if ((strcmp(rkv->rv_var, var) == 0) &&
		    (strcmp(rkv->rv_val, val) == 0))
			return 0;
	return 1;
}
