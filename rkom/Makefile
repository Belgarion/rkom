#	$Id: Makefile,v 1.14 2001/07/30 19:04:48 ragge Exp $
#
#	makefile for the rkom frontend
#
CFLAGS= -O2 -g -Wall -Werror -Wstrict-prototypes -Wmissing-prototypes
LIBS=	-L../backend -lrkom -ltermcap
INCS=	-I../backend

OBJS=	rkom.o set.o kbd.o nasta.o show.o lista.o error.o write.o \
	helpers.o parse_eng.o parse.o cmd.o container.o rprintf.o

all:	rkom

rkom: $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCS) -c $*.c

clean:
	rm -f $(OBJS) rkom *.core core
