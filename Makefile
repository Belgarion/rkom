#	$Id: Makefile,v 1.12 2002/11/26 09:01:31 ragge Exp $
#
all:
	@if [ `uname` = NetBSD -o `uname` = FreeBSD -o `uname` = Darwin -o `uname` = OpenBSD ]; then \
		echo "Using *BSD" ; \
		${MAKE} -f Makefile.bsd ; \
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
	@(cd backend; ${MAKE} clean)
	@(cd libredit; ${MAKE} clean)
	@(cd rkom; ${MAKE} clean)
