#	$Id: Makefile,v 1.8 2001/11/19 20:27:13 ragge Exp $
#
all:
	@if [ `uname` = NetBSD -o `uname` = FreeBSD ]; then \
		echo "Using *BSD" ; \
		make -f Makefile.bsd ; \
	elif [ `uname` = SunOS ]; then \
		if [ `uname -r | sed 's/\.//g'` -gt 400 ]; then \
			echo "Using SunOS4" ; \
			make -f Makefile.sunos4 ; \
		else \
			echo "Using Solaris" ; \
			make -f Makefile.solaris ; \
		fi \
	else \
		echo "Unsupported OS" ; \
		exit 1 ; \
	fi

clean:
	@(cd backend; make clean)
	@(cd libredit; make clean)
	@(cd rkom; make clean)
