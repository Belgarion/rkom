#	$Id: Makefile,v 1.13 2001/06/24 09:46:59 ragge Exp $
#
#	makefile for the rkom frontend
#
CFLAGS= -O2 -g -Wall -Werror -Wstrict-prototypes -Wmissing-prototypes
LIBS=	-L../backend -lrkom -ledit -ltermcap
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
