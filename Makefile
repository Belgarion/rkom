#	$Id: Makefile,v 1.3 2001/01/12 23:50:58 offe Exp $
#
SUBDIR=	backend rkom

.include <bsd.subdir.mk>
.ifdef __NetBSD__
.include <bsd.prog.mk>
.endif
