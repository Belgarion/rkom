#	$Id: Makefile,v 1.10 2001/11/24 20:25:53 offe Exp $
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
	elif [ `uname` = Linux ]; then \
		echo "Using Linux"; \
		make -f Makefile.linux; \
        elif [ `uname` = AIX ]; then \
                echo "Using AIX"; \
                make -f Makefile.aix; \
	else \
		echo "Unsupported OS" ; \
		exit 1 ; \
	fi

clean:
	@(cd backend; make clean)
	@(cd libredit; make clean)
	@(cd rkom; make clean)
