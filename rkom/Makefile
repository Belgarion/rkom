#	$Id: Makefile,v 1.8 2000/11/22 11:58:25 ragge Exp $
#
#	makefile for the rkom frontend
#

PROG=	rkom
SRCS=	rkom.c set.c kbd.c nasta.c show.c lista.c error.c write.c \
	helpers.c parse_eng.c parse.c cmd.c container.c rprintf.c
LDADD+= -ledit -ltermcap
NOMAN=	y
CFLAGS= -g
WARNS?=	1

.include <bsd.prog.mk>
