#	$Id: Makefile.inc,v 1.4 2001/01/28 16:40:36 ragge Exp $
#

LIBRKOM != cd ${.CURDIR}/../backend;\
    printf "xxx: .MAKE\n\t@echo \$${.OBJDIR}\n" | ${MAKE} -s -f-

CPPFLAGS+=-I${.CURDIR}/../backend -g
DPADD+= ${LIBRKOM}/librkom.a
LDADD+=	-L${LIBRKOM} -lrkom -static

#NOGCCERROR=1
#LDADD+=-L../../libedit
#CPPFLAGS+=-I../../libedit
#DEBUG=1
