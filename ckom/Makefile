#	$Id: Makefile,v 1.1 2000/10/15 11:59:39 jens Exp $
#
#	makefile for the rkom frontend
#

PROG		 = ckom
SRCS		 = ckom.c container.c articles.c scr.c conf.c
NOMAN		 = y
WARNS		?= 1
LDADD		 = -lcurses -ltermcap
CPPFLAGS	+= -DNO_E_ERR

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
