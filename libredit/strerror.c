/*	$Id: strerror.c,v 1.2 2001/11/19 20:27:23 ragge Exp $	*/

#ifdef SUNOS4

#include "rkomsupport.h"

char *
strerror(int n)
{
	static char msg[] = "Unknown error (1234567890)";

	extern int sys_nerr;
	extern char *sys_errlist[];

	if (n >= sys_nerr) {
		snprintf(msg, sizeof(msg), "Unknown error (%d)", n);
		return(msg);
	} else {
		return(sys_errlist[n]);
	}
}
#endif
