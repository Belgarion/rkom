#	$Id: Makefile,v 1.8 2001/01/06 16:05:05 ragge Exp $
#
# Rkom backend library Makefile
#

LIB=	rkom
SRCS=	rkom_subr.c rkom_lyskom.c rkom_beparse.c rkom_conf.c \
	rkom_helpers.c rkom_proto.c rkom_async.c rkom_uarea.c

MKLINT=	no	# No lint library
MKPROFILE=no	# No profiling library
MKPIC=	no	# No shared library
WARNS?=	1	# All possible warnings

SPCGEN?= spcgen

.if defined(DEBUG)
CFLAGS= -g
.endif

CLEANFILES+= rkom_proto.c rkom_proto.h rkom_proto_stubs.c

rkom_proto.c rkom_proto.h rkom_proto_stubs.c: rkom_proto.spc
	${SPCGEN} -ch -i rkom_proto.spc

.include <bsd.lib.mk>
