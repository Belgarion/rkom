
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
#include "set.h"
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
	static char *ret;
	int len;  

	if (el == NULL) {
		el = el_init("rkom", stdin, stdout, stderr);
		el_set(el, EL_EDITOR, getval("editor-mode"));
		el_set(el, EL_PROMPT, prompt_fun);
	}
	
	msg = m;
	ret = (char *)el_gets(el, &len, 0);
	if (len)
		ret[len-1] = 0;
	return ret;
}
