/*	$Id: rkom_subr.c,v 1.1 2000/09/26 18:48:00 ragge Exp $	*/
/*
 * This file contains the front-end subroutine interface.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "exported.h"
#include "backend.h"

static	int sockfd;
static	char *version = "noll";
/*
 * First connect to the server, then fork away the backend after informing
 * about our existance.
 */
int
rkom_connect(char *server, char *frontend, char *os_username)
{
	struct sockaddr_in sin;
	struct hostent *hp;
	char *buf, *buf2;

	/* Locate our KOM server */
	if ((hp = gethostbyname(server)) == NULL)
		return -1;

	/* Create a socket to play with */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	/* Connect to the server we want to talk with */
	sin.sin_family = AF_INET;
	sin.sin_port = htons(4894); /* XXX should be configureable */
	bcopy(hp->h_addr, &sin.sin_addr, hp->h_length);
	if (connect(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		return -1;

	put_char('A');
	put_string(os_username);
	put_char('\n');
	buf2 = alloca(8);
	bzero(buf2, 8);
	read(sockfd, buf2, 7);
	if (bcmp(buf2, "LysKOM\n", 7))
		return -1;

	if (frontend == NULL)
		asprintf(&buf2, "%s", "raggkom");
	else
		asprintf(&buf2, "%s (rkom backend)", frontend);
	asprintf(&buf, "69 %ldH%s %ldH%s\n", (long)strlen(buf2), buf2,
	    (long)strlen(version), version);
	send_reply(buf);
	get_accept('\n');
	free(buf);
	free(buf2);

	/* Set what async messages we want */
	send_reply("80 6 { 5 9 12 13 15 16 }\n");
	get_accept('\n');

	/* Ok, now fork the backend process */
	if (fork() == 0)
		rkom_loop(); /* Backend main loop */

	close(sockfd);
	return 0;
}
