#	$Id: Makefile,v 1.20 2003/10/10 14:08:16 ragge Exp $
#
#	makefile for the rkom frontend
#
CFLAGS= -O2 -g -Wall -Werror -Wstrict-prototypes -Wmissing-prototypes \
	$(XFLAGS) -I../support
LIBS=	-L../backend -lrkom -L../libredit -lredit -L/usr/pkg/lib # -lefence
INCS=	-I../backend -I../libredit

OBJS=	rkom.o set.o kbd.o nasta.o show.o lista.o error.o write.o \
	helpers.o parse_eng.o parse.o cmd.o container.o rprintf.o debug.o

all:	rkom

rkom: $(OBJS) ../backend/librkom.a
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(LIBS) $(XLIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCS) -c $*.c

clean:
	rm -f $(OBJS) rkom *.core core
