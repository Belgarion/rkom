/*	$Id: poll_emul.h,v 1.1 2001/11/25 20:05:37 jens Exp $	*/
/*
 * Copyright (c) 1994 Jason R. Thorpe.
 * All rights reserved.
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
 *     This product includes software developed by Jason R. Thorpe.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 */

#ifndef _POLL_EMUL_H_
#define _POLL_EMUL_H_

#include <sys/types.h>
#include <sys/cdefs.h>

#define nfds_t nfds_emul_t
typedef unsigned int nfds_emul_t;

#define pollfd pollfd_emul
struct pollfd {
	int	fd;
	short	events;
	short	revents;
};

#undef POLLIN
#undef POLLNORM
#undef POLLPRI
#undef POLLOUT
#undef POLLERR
#undef POLLHUP
#undef POLLNVAL
#undef POLLRDNORM
#undef POLLRDBAND
#undef POLLWRNORM
#undef POLLWRBAND
#undef POLLMSG

#define POLLIN		0x0001
#define POLLNORM	POLLIN
#define POLLPRI		POLLIN
#define POLLOUT		0x0008
#define POLLERR		0x0010          /* not used */
#define POLLHUP		0x0020
#define POLLNVAL	0x0040
#define POLLRDNORM	POLLIN
#define POLLRDBAND	POLLIN
#define POLLWRNORM	POLLOUT
#define POLLWRBAND	POLLOUT
#define POLLMSG		0x0800          /* not used */

#define __INVALIDPOLL  ~(POLLIN | POLLOUT)

__BEGIN_DECLS
#define poll(a,b,c) poll_emul((a),(b),(c))
int poll_emul __P((struct pollfd *, nfds_t, int));
__END_DECLS

#endif /* _POLL_H_ */
