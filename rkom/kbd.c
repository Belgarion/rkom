
#ifndef SUNOS4
#include <sys/ioctl.h>
#endif

#if !defined(AIX)
#include <termcap.h>
#endif
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <err.h>
#include <unistd.h>

#include <strings.h>
#include <string.h>

#include "rkom.h"
#include "rhistedit.h"

static char *msg;

static char *
prompt_fun(EditLine *el)
{	
	return msg;
}	
 
char *
getstr(char *m)
{
	static EditLine *el = NULL;
	char *ret;
	const char *tc;
	int len;  

	if (el == NULL) {
		el = el_init("rkom", stdin, stdout, stderr);
		el_set(el, EL_EDITOR, "emacs");
		el_set(el, EL_PROMPT, prompt_fun);
	}
	
	msg = m;
	tc = el_gets(el, &len);
	if (tc)
		ret = strdup(tc);
	else
		ret = strdup("");
	ret[len - 1] = 0; /* Forget \n */
	return ret;
}
