#	$Id: Makefile.inc,v 1.2 2000/11/18 10:35:50 ragge Exp $
#

LIBRKOM != cd ${.CURDIR}/../backend;\
    printf "xxx: .MAKE\n\t@echo \$${.OBJDIR}\n" | ${MAKE} -s -f-

CPPFLAGS+=-I${.CURDIR}/../backend -g
DPADD+= ${LIBRKOM}/librkom.a
LDADD+=	-L${LIBRKOM} -lrkom -static
