#	$Id: Makefile,v 1.3 2000/10/15 21:39:14 jens Exp $
#
#	makefile for the rkom frontend
#

PROG		 = ckom
SRCS		 = ckom.c container.c articles.c scr.c conf.c keys.c
NOMAN		 = y
WARNS		?= 1
LDADD		 = -lcurses -ltermcap

DEBUG		 = 1

.if defined(DEBUG)
LDSTATIC	 = -static
LDADD		+= -g
CFLAGS		 = -g
.endif

UNAME		!= uname
.if ${UNAME} == FreeBSD
CFLAGS		+= ${CPPFLAGS}
.endif

.include <bsd.prog.mk>
