#	$Id: Makefile,v 1.7 2001/11/18 18:23:32 ragge Exp $
#
all:
	@if [ `uname` = NetBSD -o `uname` = FreeBSD ]; then \
		echo "Using *BSD" ; \
		make -f Makefile.bsd ; \
	elif [ `uname` = SunOS -a `uname -r` = 5.8 ]; then \
		echo "Using Solaris" ; \
		make -f Makefile.solaris ; \
	fi

clean:
	@(cd backend; make clean)
	@(cd libredit; make clean)
	@(cd rkom; make clean)
