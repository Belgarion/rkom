#	$Id: Makefile,v 1.5 2000/10/09 08:33:51 ragge Exp $
#
# Rkom backend library Makefile
#

LIB=	rkom
SRCS=	rkom_subr.c rkom_lyskom.c rkom_beparse.c rkom_conf.c \
	rkom_helpers.c rkom_proto.c rkom_async.c

MKLINT=	no	# No lint library
MKPROFILE=no	# No profiling library
MKPIC=	no	# No shared library
WARNS?=	1	# All possible warnings

SPCGEN?= spcgen

CLEANFILES+= rkom_proto.c rkom_proto.h rkom_proto_stubs.c

rkom_proto.c rkom_proto.h rkom_proto_stubs.c: rkom_proto.spc
	${SPCGEN} -chs -i rkom_proto.spc

.include <bsd.lib.mk>
