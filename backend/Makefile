#	$Id: Makefile,v 1.17 2003/09/17 10:51:34 ragge Exp $
#
# Rkom backend library Makefile
#
CFLAGS=	-O2 -g -Wall -Wstrict-prototypes -Wmissing-prototypes \
	$(XFLAGS) -I../support -I../libredit

OBJS=	rkom_subr.o rkom_lyskom.o rkom_beparse.o rkom_conf.o \
	rkom_helpers.o rkom_async.o rkom_uarea.o \
	$(EXTRA_OBJS)

all:	librkom.a

librkom.a: $(OBJS)
	ar cr librkom.a $(OBJS)
	ranlib librkom.a

clean:
	rm -f $(OBJS) librkom.a

.c.o:
	$(CC) $(CFLAGS) -c $*.c
