
#include <sys/ioctl.h>

#include <termcap.h>
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
	EditLine *el;
	char *ret;
	const char *tc;
	int len;  
	
	msg = m;
#if !defined(__FreeBSD__)
	el = el_init("rkom", stdin, stdout, stderr);
#else
	el = el_init("rkom", stdin, stdout);
#endif
	el_set(el, EL_EDITOR, "emacs");
	el_set(el, EL_PROMPT, prompt_fun);
	tc = el_gets(el, &len);
	if (tc)
		ret = strdup(tc);
	else
		ret = strdup("");
	ret[len - 1] = 0; /* Forget \n */
	el_end(el);
	return ret;
}
