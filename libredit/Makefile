#

CFLAGS=	-O2 -g -Wall -Werror -Wstrict-prototypes -Wmissing-prototypes $(XFLAGS)

AR=	ar
RANLIB=	ranlib

OBJS=	chared.o common.o el.o emacs.o fcns.o help.o hist.o key.o map.o \
	parse.o prompt.o read.o refresh.o search.o sig.o term.o tty.o vi.o \
	tokenizer.o history.o fgetln.o strvis.o strunvis.o strsep.o

all:	libredit.a

libredit.a: ${OBJS}
	${AR} cr $@ ${OBJS}
	${RANLIB} $@

.c.o:
	$(CC) $(CFLAGS) $(INCS) -c $*.c

clean:
	rm -f libredit.a *.core ${OBJS}
	rm -f common.h emacs.h fcns.c fcns.h help.c help.h vi.h

vi.h: vi.c
	sh makelist -h vi.c > $@

emacs.h: emacs.c
	sh makelist -h emacs.c > $@

common.h: common.c
	sh makelist -h common.c > $@

fcns.h: vi.h emacs.h common.h
	sh makelist -fh vi.h emacs.h common.h > $@

fcns.c: vi.h emacs.h common.h fcns.h
	sh makelist -fc vi.h emacs.h common.h > $@

help.c: vi.c emacs.c common.c
	sh makelist -bc vi.c emacs.c common.c > $@

help.h: vi.c emacs.c common.c
	sh makelist -bh vi.c emacs.c common.c > $@

${OBJS}: common.h emacs.h fcns.c fcns.h help.c help.h vi.h
