#	$Id: Makefile,v 1.5 2001/11/18 14:35:11 ragge Exp $
#
all:
	@if [ `uname` = NetBSD ]; then \
		echo "Using NetBSD" ; \
		make -f Makefile.bsd ; \
	elif [ `uname` = SunOS ]; then \
		make -f Makefile.sunos ; \
	fi

clean:
	@(cd backend; make clean)
	@(cd libredit; make clean)
	@(cd rkom; make clean)
