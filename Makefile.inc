#	$Id: Makefile.inc,v 1.3 2000/12/29 11:55:38 jens Exp $
#

LIBRKOM != cd ${.CURDIR}/../backend;\
    printf "xxx: .MAKE\n\t@echo \$${.OBJDIR}\n" | ${MAKE} -s -f-

CPPFLAGS+=-I${.CURDIR}/../backend -g
DPADD+= ${LIBRKOM}/librkom.a
LDADD+=	-L${LIBRKOM} -lrkom -static

#DEBUG=1
