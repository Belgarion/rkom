/*	$NetBSD: sys.h,v 1.4 2000/09/04 22:06:32 lukem Exp $	*/

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
 *
 *	@(#)sys.h	8.1 (Berkeley) 6/4/93
 */

/*
 * sys.h: Put all the stupid compiler and system dependencies here...
 */
#ifndef _h_sys
#define	_h_sys

#include <sys/param.h>

#include <termios.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>


#ifndef public
# define public		/* Externally visible functions/variables */
#endif

#ifndef private
# define private	static	/* Always hidden internals */
#endif

#ifndef protected
# define protected	/* Redefined from elsewhere to "static" */
			/* When we want to hide everything	*/
#endif

#ifndef _PTR_T
# define _PTR_T
typedef void	*ptr_t;
#endif

#ifndef _IOCTL_T
# define _IOCTL_T
typedef void	*ioctl_t;
#endif

#if defined(SOLARIS) || defined(SUNOS4) || defined(LINUX)
/* fgetln() stdio routine missing */
char	*fgetln(FILE *, size_t *);
#endif

#if defined(SOLARIS) || defined(SUNOS4) || defined(LINUX)
/* vis() soutines missing */
#define VIS_SP          0x04    /* also encode space */
#define VIS_TAB         0x08    /* also encode tab */
#define VIS_NL          0x10    /* also encode newline */
#define VIS_WHITE       (VIS_SP | VIS_TAB | VIS_NL)
#define VIS_SAFE        0x20    /* only encode "unsafe" characters */

#define	VIS_WHITE	(VIS_SP | VIS_TAB | VIS_NL)

#define VIS_NOSLASH     0x40    /* inhibit printing '\' */
#define VIS_OCTAL       0x01    /* use octal \ddd format */
#define VIS_CSTYLE      0x02    /* use \[nrft0..] where appropiate */

/*
 * unvis return codes
 */
#define UNVIS_VALID      1      /* character valid */
#define UNVIS_VALIDPUSH  2      /* character valid, push back passed char */
#define UNVIS_NOCHAR     3      /* valid sequence, no character produced */
#define UNVIS_SYNBAD    -1      /* unrecognized escape sequence */
#define UNVIS_ERROR     -2      /* decoder in unknown state (unrecoverable) */
#define UNVIS_END       1       /* no more characters */
char    *vis(char *, int, int, int);
int     strvis(char *, const char *, int);
int     strvisx(char *, const char *, size_t, int);
int     strunvis(char *, const char *);
int     unvis(char *, int, int *, int);
#endif

#ifdef SUNOS4
#include <memory.h>
#endif

#if defined(SUNOS4) || defined(LINUX)
size_t strlcpy(char *dst, const char *src, size_t size);
size_t strlcat(char *dst, const char *src, size_t size);
#endif

#endif /* _h_sys */
