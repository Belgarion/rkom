#	$Id: Makefile.inc,v 1.5 2001/03/17 00:02:24 offe Exp $
#

LIBRKOM != cd ${.CURDIR}/../backend;\
    printf "xxx: .MAKE\n\t@echo \$${.OBJDIR}\n" | ${MAKE} -s -f-

CPPFLAGS+=-I${.CURDIR}/../backend -g
DPADD+= ${LIBRKOM}/librkom.a
LDADD+=	-L${LIBRKOM} -lrkom -static

LIBDIR= /usr/local/lib
BINDIR= /usr/local/bin
MANDIR= /usr/local/man

#NOGCCERROR=1
#LDADD+=-L../../libedit
#CPPFLAGS+=-I../../libedit
#DEBUG=1
