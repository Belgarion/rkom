#	$Id: Makefile,v 1.4 2001/06/24 09:46:47 ragge Exp $
#
all:
	@(cd backend; make)
	@(cd rkom; make)

clean:
	@(cd backend; make clean)
	@(cd rkom; make clean)
