/*	$Id: rkom_subr.c,v 1.3 2000/10/07 10:38:46 ragge Exp $	*/
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

#include "exported.h"
#include "protocol.h"
#include "backend.h"

int sockfd, readfd, writefd, async;
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

	pipe(toback);
	pipe(fromback);
	pipe(asyncio);

	/* Ok, now fork the backend process */
	if ((childpid = fork()) == 0) {
		close(0);
		close(toback[1]);
		close(fromback[0]);
		close(asyncio[0]);
		writefd = fromback[1];
		readfd = toback[0];
		async = asyncio[1];
		rkom_loop(); /* Backend main loop */
	}

	writefd = toback[1];
	readfd = fromback[0];
	async = asyncio[0];
	close(toback[0]);
	close(fromback[1]);
	close(asyncio[1]);
	close(sockfd);
	return 0;
}

int
rkom_matchconf(char *user, int flags, struct confinfo **matched)
{
	struct confinfo *ret;
	char *reply, *send;
	int cnt, len, i, j;

	i = strlen(user) + 1 + sizeof(int);
	send = alloca(i);
	strcpy(send, user);
	bcopy(&flags, &send[strlen(user) + 1], sizeof(int));
	fgrw(MATCHUSER, send, i, (void **)&reply, &len);
	for (cnt = i = 0; i < len; i++)
		if (reply[i] == 0) {
			cnt++;
			i += 2 * sizeof(int);
		}

	ret = malloc((cnt + 1) * sizeof(struct confinfo));
	for (j = i = 0; j < cnt; j++) {
		ret[j].name = &reply[i];
		i += strlen(ret[j].name) + 1;
		bcopy(&reply[i], &ret[j].type, sizeof(int));
		i += sizeof(int);
		bcopy(&reply[i], &ret[j].conf_no, sizeof(int));
		i += sizeof(int);
	}
	*matched = ret;
	return cnt;
}

int
rkom_login(int userid, char *passwd)
{
	char *arg;
	int len;

	len = strlen(passwd) + 1 + sizeof(int);
	arg = alloca(len);
	strcpy(arg, passwd);
	bcopy(&userid, &arg[strlen(passwd) + 1], sizeof(int));
	return fgrw(LOGIN, arg, len, 0, 0);
}

void
rkom_logout()
{
	kill(childpid, SIGTERM);
	err(1, "rkom_logout");
}

void
rkom_alive()
{
	fgrw(ALIVE, 0, 0, 0, 0);
}

void
rkom_time(struct tm *tm)
{
	void *reply;

	fgrw(TIME, 0, 0, &reply, 0);
	bcopy(reply, tm, sizeof(struct tm));
	free(reply);
}

int
rkom_whatido(char *str)
{
	return fgrw(WHATIDO, str, strlen(str) + 1, 0, 0);
}

int
rkom_unreadconf(int mid, int **confs, int *nconf)
{
	int hej;

	hej = fgrw(UNREADCONF, &mid, sizeof(int), (void **)confs, nconf);
	*nconf /= sizeof(int);
	return hej;
}
	
int
rkom_confinfo(int mid, struct conference **conf)
{
	struct conference *c;
	char *string;
	int len, i;

	i = fgrw(CONFINFO, &mid, sizeof(int), (void **)&string, &len);
	if (i)
		return i;

	c = (struct conference *)string;
	c->name = string + sizeof(struct conference);
	/* XXX aux-item unhandled so far */
	*conf = c;
	return 0;
}


int
rkom_membership(int uid, int conf, struct membership **members)
{
	struct membership *m;
	char *buf, *string;
	int i;

	buf = alloca(2*sizeof(int));
	bcopy(&uid, buf, sizeof(int));
	bcopy(&conf, buf + sizeof(int), sizeof(int));
	i = fgrw(MEMBERSHIP, buf, 2*sizeof(int), (void **)&string, 0);
	if (i)
		return i;

	m = (struct membership *)string;
	m->read_texts = (int *)(string + sizeof(struct membership));
	*members = m;
	return 0;
}

void
rkom_who(int secs, int flags, struct dynamic_session_info **info)
{
	struct dynamic_session_info *inf;
	char *buf, *string;
	int end, i;

	buf = alloca(2*sizeof(int));
	bcopy(&secs, buf, sizeof(int));
	bcopy(&flags, buf + sizeof(int), sizeof(int)); 
	fgrw(WHO, buf, 2*sizeof(int), (void **)&string, 0);
	/* The last struct comes with a session number of zero */
	inf = (struct dynamic_session_info *)string;
	for (end = 0; inf[end].session; end++)
		;
	buf = (char *)&inf[end + 1];
	for (i = 0; i < end; i++) {
		inf[end].doing = buf;
		buf += strlen(inf[end].doing) + 1;
	}
	*info = inf;
}

int
rkom_persinfo(int uid, struct person **person)
{
	struct person *p;
	char *c;
	int ret;

	ret = fgrw(PERSINFO, &uid, sizeof(int), (void **)&c, 0);
	if (ret)
		return ret;

	p = (struct person *)c;
	p->username = c + sizeof(struct person);
	*person = p;
	return 0;
}





