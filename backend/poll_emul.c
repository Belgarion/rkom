/*	$Id: poll_emul.c,v 1.1 2001/11/25 20:05:37 jens Exp $	*/
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

/*
 * Approximate the System V system call ``poll()'' with select(2).
 */

#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include "poll_emul.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef __APPLE__
static
#endif
struct timeval *misc_poll_gettimeout __P((int, int *, struct timeval *));

int
poll_emul(fds, nfds, timeout)
       struct pollfd *fds;
       nfds_t nfds;
       int timeout;
{
       fd_set reads, writes;
       int i, inval, rval, tsize, maxfd;
       struct timeval *tv, stv;

       tsize = (int)sysconf(_SC_OPEN_MAX);
       if (tsize > FD_SETSIZE)
               tsize = FD_SETSIZE;

	/*
	 * Sanity check
	 */
	if (fds == NULL && nfds > 0) {
		errno = EINVAL;
		return -1;
	}

       /*
        * Special case of a simple timeout with no
        * event checking.
        */
       if (nfds == 0) {
               tv = misc_poll_gettimeout(timeout, &rval, &stv);
               if (rval == -1) {
                       errno = EAGAIN;
                       return (-1);
               }
               return (select(tsize, 0, 0, 0, tv));
       }

       /* initialize the fd_sets and invalids */
       FD_ZERO(&reads);
       FD_ZERO(&writes);
       inval = 0;

       /*
        * Run through the file descriptors and:
        * a.  clear revents field
        * b.  check for validity
        * c.  check event and place in appropriate fd_set
	* d.  record the largest descriptor
        */
       for (maxfd = i = 0; i < nfds; i++) {
               /* a.  clear revents field */
               fds[i].revents = 0;

		/* Disabled descriptor */
		if (fds[i].fd == -1)
			continue;

               /* b.  check validity */
               if (fcntl(fds[i].fd, F_GETFL, O_NDELAY) == -1) {
                       fds[i].revents |= POLLNVAL;
                       ++inval;
                       continue;
               }

		if (fds[i].events == 0)
			continue;

               /* c.  check event and add */
               if (fds[i].events & __INVALIDPOLL) {
                       errno = EINVAL;
                       return (-1);
               }

               if (fds[i].events & POLLIN)
                       FD_SET(fds[i].fd, &reads);

               if (fds[i].events & POLLOUT)
                       FD_SET(fds[i].fd, &writes);

		if (fds[i].fd > maxfd)
			maxfd = fds[i].fd;
       }

       /*
	* If maxfd is larger than the number of possible file
	* descriptors simply return an error condition now.
        */
       if (maxfd > tsize) {
               errno = EINVAL;
               return (-1);
       }

       tv = misc_poll_gettimeout(timeout, &rval, &stv);
       if (rval == -1) {
               errno = EAGAIN;
               return (-1);
       }

       /*
        * Call select and loop through the descriptors again, checking
        * for read and write events.
        */
       if (inval == nfds)
               return (inval);

       errno = 0;

       rval = select(maxfd + 1, &reads, &writes, 0, tv);
       if (rval < 1)           /* timeout or error condition */
               return (rval);  /* errno will be set by select() */
               
       rval = 0;
       for (i = 0; i < nfds; i++) {
               if (fds[i].revents & POLLNVAL || fds[i].fd == -1)
                       continue;

               if (FD_ISSET(fds[i].fd, &reads))
                       fds[i].revents |= POLLIN;

               if (FD_ISSET(fds[i].fd, &writes))
                       fds[i].revents |= POLLOUT;

               /*
                * XXX This is only an approximation.  I have no idea
                * how accurate this might be.
                */
               if (fcntl(fds[i].fd, F_GETFL, O_NDELAY) == -1)
                       fds[i].revents |= POLLHUP;

               if (fds[i].revents != 0)
                       ++rval;
       }
       rval += inval;

       return (rval);
}

#ifndef __APPLE__
static
#endif
struct timeval *
misc_poll_gettimeout(timeout, rval, tv)
	int timeout;
	int *rval;
	struct timeval *tv;
{
       if (timeout < 0) {
               *rval = 0;
               return (NULL);
       } else {
               tv->tv_usec = timeout * 1000;
               tv->tv_sec = tv->tv_usec / 1000000;
               tv->tv_usec %= 1000000;

               return (tv);
       }
}
