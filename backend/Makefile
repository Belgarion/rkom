#	$Id: Makefile,v 1.16 2002/07/14 17:08:58 offe Exp $
#
# Rkom backend library Makefile
#
CFLAGS=	-O2 -g -Wall -Wstrict-prototypes -Wmissing-prototypes \
	$(XFLAGS) -I../support -I../libredit

OBJS=	rkom_subr.o rkom_lyskom.o rkom_beparse.o rkom_conf.o \
	rkom_helpers.o rkom_proto.o rkom_async.o rkom_uarea.o \
	$(EXTRA_OBJS)

all:	librkom.a

librkom.a: rkom_proto.h $(OBJS)
	ar cr librkom.a $(OBJS)
	ranlib librkom.a

rkom_proto.c rkom_proto.h rkom_proto_stubs.c: rkom_proto.spc
	spcgen -ch -i rkom_proto.spc
	echo '#include "rkomsupport.h"' > rkom_proto.c.tmp
	cat rkom_proto.c >> rkom_proto.c.tmp
	mv rkom_proto.c.tmp rkom_proto.c

clean:
	rm -f $(OBJS) librkom.a

.c.o:
	$(CC) $(CFLAGS) -c $*.c
