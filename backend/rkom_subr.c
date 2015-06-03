/*	$Id: rkom_subr.c,v 1.25 2003/09/25 18:38:17 ragge Exp $	*/
/*
 * This file contains the front-end subroutine interface.
 */
#ifdef SOLARIS
#undef _XPG4_2
#endif

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <signal.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <err.h>

#include "rkomsupport.h"
#include "backend.h"

FILE *sfd;
int pfd;
int komerr;

/*
 * First connect to the server, then fork away the backend after informing
 * about our existance.
 */
struct rk_server *
rkom_connect(char *server, char *frontend, char *os_username, char *fevers)
{
	static struct rk_server rs;
	struct addrinfo hints, *res, *ressave;
	int n;
	char *buf2;

	komerr = -1; /* XXX */

	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	/* Locate our KOM server */
	n = getaddrinfo(server, "4894", &hints, &res);
	if (n < 0) {
		return NULL;
	}

	ressave = res;

	// Try all possible IP/IPv6 addresses
	while (res) {
		/* Create a socket to play with */
		if ((pfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
			continue;

		/* Connect to the server we want to talk with */
		if (connect(pfd, res->ai_addr, res->ai_addrlen) == 0) {
			// Connection successful
			break;
		}

		close(pfd);
		pfd = -1;

		res = res->ai_next;
	}
	freeaddrinfo(ressave);
	if (pfd < 0) {
		// Connection failed
		return NULL;
	}

	if ((sfd = fdopen(pfd, "w")) == NULL)
		return NULL;
	setvbuf(sfd, (char *)NULL, _IOLBF, 0);

	put_char('A');
	put_string(os_username);
	put_char('\n');
	buf2 = alloca(8);
	bzero(buf2, 8);

	read(pfd, buf2, 7);
	if (bcmp(buf2, "LysKOM\n", 7))
		return NULL;

	if (*frontend == 0) {
		buf2 = alloca(20);
		sprintf(buf2, "%s", "raggkom");
	} else {
		buf2 = alloca(strlen(frontend) + 30);
		sprintf(buf2, "%s (rkom backend)", frontend);
	}
	send_reply("69 %ldH%s %ldH%s\n", (long)strlen(buf2), buf2,
	    (long)strlen(fevers), fevers);
	get_accept('\n');

	/* Check what the server is running */
	send_reply("75\n");
	rs.rs_proto = get_int();
	rs.rs_servtype = get_string();
	rs.rs_version = get_string();
	get_accept('\n');

	/* Set what async messages we want */
	send_reply("80 10 { 5 8 9 12 13 14 15 16 17 18 }\n");
	get_accept('\n');

	return &rs;
}

void
rkom_logout()
{
	exit(0);
}
