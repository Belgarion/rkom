#	$Id: Makefile,v 1.12 2001/01/15 23:47:56 offe Exp $
#
#	makefile for the rkom frontend
#

PROG=	rkom
SRCS=	rkom.c set.c kbd.c nasta.c show.c lista.c error.c write.c \
	helpers.c parse_eng.c parse.c cmd.c container.c rprintf.c
LDADD+= -ledit -ltermcap
NOMAN=	y
CFLAGS+= -g -I../backend
WARNS?=	1

.if defined(DEBUG)
LDADD+= -L/usr/pkg/lib -lefence
.endif

.include <bsd.prog.mk>
