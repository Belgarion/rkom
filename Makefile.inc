#	$Id: Makefile.inc,v 1.1 2000/09/26 18:48:00 ragge Exp $
#

LIBRKOM != cd ${.CURDIR}/../backend;\
    printf "xxx: .MAKE\n\t@echo \$${.OBJDIR}\n" | ${MAKE} -s -f-

CPPFLAGS+=-I${.CURDIR}/../backend
DPADD+= ${LIBRKOM}/librkom.a
LDADD+=	-L${LIBRKOM} -lrkom
