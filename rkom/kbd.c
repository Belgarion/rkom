
#include <strings.h>
#include <histedit.h>

#include "rkom.h"

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
	int len;  
	
	msg = m;
#if !defined(__FreeBSD__)
	el = el_init("rkom", stdin, stdout, stderr);
#else
	el = el_init("rkom", stdin, stdout);
#endif
	el_set(el, EL_EDITOR, "emacs");
	el_set(el, EL_PROMPT, prompt_fun);
	ret = strdup(el_gets(el, &len));
	ret[len - 1] = 0; /* Forget \n */
	el_end(el);
	return ret;
}
