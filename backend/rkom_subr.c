/*	$Id: rkom_subr.c,v 1.7 2000/10/25 09:40:24 ragge Exp $	*/
/*
 * This file contains the front-end subroutine interface.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <signal.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <err.h>

#include "rkom_proto.h"
#include "exported.h"
#include "backend.h"

int sockfd, readfd, writefd, asyncfd, fepid;
static char *version = "noll";
static int childpid;
/*
 * First connect to the server, then fork away the backend after informing
 * about our existance.
 */
int
rkom_connect(char *server, char *frontend, char *os_username)
{
	struct sockaddr_in sin;
	struct hostent *hp;
	int toback[2], fromback[2], asyncio[2];
	char *buf, *buf2;
	int ver;

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

	/* Check what the server is running */
	send_reply("75\n");
	ver = get_int();
	buf = get_string();
	buf2 = get_string();
	get_accept('\n');

	printf("Välkommen till raggkom kopplad till server %s.\n", server);
	printf("Servern kör %s, version %s. Protokollversion %d.\n", 
	    buf, buf2, ver);
	free(buf);free(buf2);
	if (ver < 10) {
		printf("\nVARNING: Protokollversionen bör vara minst 10.\n");
		printf("VARNING: Vissa saker kan ofungera.\n");
	}

	/* Set what async messages we want */
	send_reply("80 10 { 5 8 9 12 13 14 15 16 17 18 }\n");
	get_accept('\n');

	pipe(toback);
	pipe(fromback);
	pipe(asyncio);
	fepid = getpid();

	/* Ok, now fork the backend process */
	if ((childpid = fork()) == 0) {
		close(0);
		close(toback[1]);
		close(fromback[0]);
		close(asyncio[0]);
		spc_set_write_fd(fromback[1]);
		spc_set_read_fd(toback[0]);
		writefd = fromback[1];
		readfd = toback[0];
		asyncfd = asyncio[1];
		rkom_loop(); /* Backend main loop */
	}

	spc_set_write_fd(toback[1]);
	spc_set_read_fd(fromback[0]);

	writefd = toback[1];
	readfd = fromback[0];
	asyncfd = asyncio[0];
	close(toback[0]);
	close(fromback[1]);
	close(asyncio[1]);
	close(sockfd);
	return 0;
}

void
rkom_logout()
{
	kill(childpid, SIGTERM);
	err(1, "rkom_logout");
}
