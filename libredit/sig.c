/*	$NetBSD: sig.c,v 1.6 2000/09/04 22:06:32 lukem Exp $	*/

/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Christos Zoulas of Cornell University.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * sig.c: Signal handling stuff.
 *	  our policy is to trap all signals, set a good state
 *	  and pass the ball to our caller.
 */
#include "sys.h"
#include "el.h"

#ifdef SOLARIS
#undef _XPG4_2
#endif
#include <signal.h>

#if defined(SOLARIS) || defined(SUNOS4)
#undef SIG_ERR  /* XXX - bad system headers */
#define SIG_ERR (void(*)(int))-1
#endif

private EditLine *sel = NULL;

private int sighdl[] = {
#define	_DO(a)	(a),
	ALLSIGS
#undef	_DO
	- 1
};

private void sig_handler(int);

/* sig_handler():
 *	This is the handler called for all signals
 *	XXX: we cannot pass any data so we just store the old editline
 *	state in a private variable
 */
private void
sig_handler(int signo)
{
	int i;
	sigset_t nset, oset;

	(void) sigemptyset(&nset);
	(void) sigaddset(&nset, signo);
	(void) sigprocmask(SIG_BLOCK, &nset, &oset);

	switch (signo) {
	case SIGCONT:
		tty_rawmode(sel);
		if (ed_redisplay(sel, 0) == CC_REFRESH)
			re_refresh(sel);
		term__flush();
		break;

	case SIGWINCH:
		el_resize(sel);
		break;

	default:
		tty_cookedmode(sel);
		break;
	}

	for (i = 0; sighdl[i] != -1; i++)
		if (signo == sighdl[i])
			break;

	(void) xsignal_restart(signo, sel->el_signal[i], 1);
	(void) sigprocmask(SIG_SETMASK, &oset, NULL);
	(void) kill(0, signo);
}


/* sig_init():
 *	Initialize all signal stuff
 */
protected int
sig_init(EditLine *el)
{
	int i;
	sigset_t nset, oset;

	(void) sigemptyset(&nset);
#define	_DO(a) (void) sigaddset(&nset, SIGWINCH);
	ALLSIGS
#undef	_DO
	    (void) sigprocmask(SIG_BLOCK, &nset, &oset);

#define	SIGSIZE (sizeof(sighdl) / sizeof(sighdl[0]) * sizeof(sigfunc))

	el->el_signal = (sigfunc *) el_malloc(SIGSIZE);
	for (i = 0; sighdl[i] != -1; i++)
		el->el_signal[i] = SIG_ERR;

	(void) sigprocmask(SIG_SETMASK, &oset, NULL);

	return (0);
}


/* sig_end():
 *	Clear all signal stuff
 */
protected void
sig_end(EditLine *el)
{

	el_free((ptr_t) el->el_signal);
	el->el_signal = NULL;
}


/* sig_set():
 *	set all the signal handlers
 */
protected void
sig_set(EditLine *el)
{
	int i;
	sigset_t nset, oset;

	(void) sigemptyset(&nset);
#define	_DO(a) (void) sigaddset(&nset, SIGWINCH);
	ALLSIGS
#undef	_DO
	    (void) sigprocmask(SIG_BLOCK, &nset, &oset);

	for (i = 0; sighdl[i] != -1; i++) {
		sigfunc s;
		/* This could happen if we get interrupted */
		if ((s = xsignal_restart(sighdl[i], sig_handler, 1)) != sig_handler)
			el->el_signal[i] = s;
	}
	sel = el;
	(void) sigprocmask(SIG_SETMASK, &oset, NULL);
}


/* sig_clr():
 *	clear all the signal handlers
 */
protected void
sig_clr(EditLine *el)
{
	int i;
	sigset_t nset, oset;

	(void) sigemptyset(&nset);
#define	_DO(a) (void) sigaddset(&nset, SIGWINCH);
	ALLSIGS
#undef	_DO
	    (void) sigprocmask(SIG_BLOCK, &nset, &oset);

	for (i = 0; sighdl[i] != -1; i++)
		if (el->el_signal[i] != SIG_ERR)
			(void) xsignal_restart(sighdl[i], el->el_signal[i], 1);

	sel = NULL;		/* we are going to die if the handler is
				 * called */
	(void) sigprocmask(SIG_SETMASK, &oset, NULL);
}

/*
 * Install a POSIX signal handler, allowing the invoker to set whether
 * the signal should be restartable or not
 */
sigfunc
xsignal_restart(int sig, sigfunc func, int restartable)
{
#ifdef ultrix   /* XXX: this is lame - how do we test sigvec vs. sigaction? */
	struct sigvec vec, ovec;

	vec.sv_handler = func;
	sigemptyset(&vec.sv_mask);
	vec.sv_flags = 0;
	if (sigvec(sig, &vec, &ovec) < 0)
		return (SIG_ERR);
	return (ovec.sv_handler);
#else   /* ! ultrix */
	struct sigaction act, oact;
	act.sa_handler = func;

	sigemptyset(&act.sa_mask);
#if defined(SA_RESTART)                 /* 4.4BSD, Posix(?), SVR4 */
	act.sa_flags = restartable ? SA_RESTART : 0;
#elif defined(SA_INTERRUPT)             /* SunOS 4.x */
	act.sa_flags = restartable ? 0 : SA_INTERRUPT;
#else
#error "system must have SA_RESTART or SA_INTERRUPT"
#endif
	if (sigaction(sig, &act, &oact) < 0)
		return (SIG_ERR);
	return (oact.sa_handler);
#endif  /* ! ultrix */
}

