#	$Id: Makefile,v 1.9 2001/06/24 09:46:56 ragge Exp $
#
# Rkom backend library Makefile
#
CFLAGS=	-O2 -g -Wall -Werror -Wstrict-prototypes -Wmissing-prototypes

OBJS=	rkom_subr.o rkom_lyskom.o rkom_beparse.o rkom_conf.o \
	rkom_helpers.o rkom_proto.o rkom_async.o rkom_uarea.o

all:	librkom.a

librkom.a: rkom_proto.h $(OBJS)
	ar cq librkom.a $(OBJS)
	ranlib librkom.a

rkom_proto.c rkom_proto.h rkom_proto_stubs.c: rkom_proto.spc
	spcgen -ch -i rkom_proto.spc

clean:
	rm -f $(OBJS) librkom.a

.c.o:
	$(CC) $(CFLAGS) -c $*.c
