#	$Id: Makefile,v 1.7 2000/11/05 16:26:19 jens Exp $
#
#	makefile for the rkom frontend
#

PROG=	rkom
SRCS=	rkom.c set.c kbd.c nasta.c show.c lista.c error.c write.c \
	helpers.c parse_eng.c parse.c cmd.c container.c
LDADD+= -ledit -ltermcap
NOMAN=	y
CFLAGS= -g
WARNS?=	1

.include <bsd.prog.mk>
