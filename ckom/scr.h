/* $Id: scr.h,v 1.1 2000/10/15 11:59:39 jens Exp $ */
#ifndef SCR_H
#define SCR_H

extern WINDOW *cmdwin;

void scr_start __P((void));
void scr_cleanup __P((void));

int scr_read_prompt __P((WINDOW *, const char *prompt, char *, int));

void scr_errx __P((const char *fmt, ...))
	__attribute__ ((format (printf, 1, 2)));
void scr_warnx __P((WINDOW *win, const char *fmt, ...))
	__attribute__ ((format (printf, 2, 3)));

#endif /* SCR_H */
