
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

/* added by ian 15:29 11/2 2000 */
#ifndef INFTIM
#define INFTIM -1
#endif

int	main(int, char **);
static	void prerr(char, char);

static	int sockfd;
static	int wait_for_reply_no = 0;
static	int reqnr = 1;
static	int unget;
static	int lastcmd;

static	char *version = "noll";

#define	CMD_ENAB	1
#define	CMD_USER	2
static	struct {
	char *name;
	int num;
} cmds[] = {
	"ENAB ", CMD_ENAB,
	"USER ", CMD_USER,
	0
};

static int
getcmd(void)
{
	char buf[21];
	int i, len;

	bzero(buf, sizeof(buf));
	i = read(0, buf, 20);
	if (i != 20)
		errx(1, "cmd len %d != 20", i);
	len = atoi(&buf[5]);
	if (len <= 0)
		errx(1, "len error");
	for (i = 0; cmds[i].name; i++)
		if (bcmp(cmds[i].name, buf, 5) == 0)
			break;
	if (cmds[i].name == 0)
		errx(1, "protocol error");
	lastcmd = i;
	return len;
}

static char *
getstring(int len)
{
	char *str;
	int i;

	str = malloc(len);
	i = read(0, str, len);
	if (i != len)
		errx(1, "string len error");
	return str;
}

static char *
getnext(char *str)
{
	char *ny;
	int len;

	len = strlen(str);
	ny = &str[len + 1];
	return ny;
}

int
main(int argc, char *argv[])
{
	struct sockaddr_in sin;
	struct hostent *hp;
	int ch;
	char *server, buf[100], *uname, buf2[100];


	len = getcmd();
	if (lastcmd != CMD_ENAB)
		errx(1, "command != ENAB");

	server = getstring(len);
	uname = getnext(server);
	frontend = getnext(uname);

	/*
	 * Locate our KOM server.
	 */
	if ((hp = gethostbyname(server)) == NULL)
		errx(1, "unknown host %s", server);

	/*
	 * Create a socket to play with.
	 */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err(2, "create socket");

	/*
	 * Connect to the server we want to talk with.
	 */
	sin.sin_family = AF_INET;
	sin.sin_port = htons(4894); /* XXX should be configureable */
	bcopy(hp->h_addr, &sin.sin_addr, hp->h_length);
	if (connect(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		err(3, "connect to server");

	/*
	 * Check protocol type; so that we know this is server is useable.
	 */
	put_char('A');
	put_string(uname);
	put_char('\n');
	bzero(buf, 10);
	read(sockfd, buf, 7);
	if (bcmp(buf, "LysKOM\n", 7))
		errx(4, "server speaking wrong protocol");

	/* Tell the world what we are using */
	if (*frontend == 0)
		bcopy("raggkom", buf2, 8);
	else
		sprintf(buf2, "%s (rkom backend)", frontend);
	sprintf(buf, "69 %ldH%s %ldH%s\n", (long)strlen(buf2), buf2,
	    (long)strlen(version), version);
	send_reply(buf);
	get_accept('\n');

	/* Set what async messages we want */
	send_reply("80 6 { 5 9 12 13 15 16 }\n");
	get_accept('\n');
	/*
	 * Ok, everything is now OK. Announce ourself. Set default prompt.
	 */
	handle();
	/* NOTREACHED */
	return 0; /* XXX GCC */
}

static int level = -1;
static int lasttime;

int
handle()
{
	struct timeval tp;
	struct pollfd pfd[2];
	int err = 0, pollkoll = 0, showprompt;

	level++;
	if (level == 0)
		pollkoll = POLLIN|POLLPRI;
	/*
	 * Set up the poll descriptors.
	 */
	pfd[0].fd = 0;
	pfd[0].events = pollkoll;
	pfd[1].fd = sockfd;
	pfd[1].events = POLLIN|POLLPRI;

	/*
	 * Ok, everything seems OK. Get into the poll loop and wait
	 * for things to happen.
	 */
	for (;;) {
		int rv;
	
		showprompt = 0;
		/* First check if there are any unhandled async messages */
		async_handle(level);

		/* Wait for something to happen */
		rv = poll(pfd, 2, INFTIM);
		if (rv == 0)
			continue;
		if (rv < 0) {
			if (errno != EINTR)
				warn("poll");
			continue;
		}
		if (pfd[0].revents & pollkoll) {
			kbd_input(pfd[0].fd);
			showprompt++;
			gettimeofday(&tp, 0);
			if (tp.tv_sec - lasttime > 30) {
				send_reply("82\n");
				get_accept('\n');
			}
		}
		if (pfd[1].revents & (POLLIN|POLLPRI)) {
			int i;
			char c = get_char();

			switch (c) {
			case ':':
				if (async(level))
					showprompt++;
				break;

			case '%':
				c = get_char();
				if (c == '%') {
					printf("Obra msg från servern: ");
					while (c != '\n') {
						c = get_char();
						putchar(c);
					}
					errx(56, "Hejdå!");
				}
				unget = c;
				err = 1;
				/* FALLTHROUGH */
			case '=':
				i = get_int();
				if (wait_for_reply_no == i) {
					wait_for_reply_no = 0;
					level--;
					return err;
				} else {
					printf("Okänd reply ");
					while (c != '\n') {
						c = get_char();
						putchar(c);
					}
				}
				break;

			default:
				printf("Okänt msg från servern: %c", c);
				while (c != '\n') {
					c = get_char();
					putchar(c);
				}
				break;
			}
		}
		if ((level == 0) && showprompt) {
			printf("\n%s - ", prompt);
			fflush(stdout);
		}
	}
}

static int in_state = 0;

int
send_reply(msg)
	char *msg;
{
	char buf[12];

	if (in_state == 0) {
		wait_for_reply_no = reqnr;
		sprintf(buf, "%d ", reqnr++);
		write(sockfd, buf, strlen(buf));
	}
	write(sockfd, msg, strlen(msg));
	in_state = (msg[strlen(msg) - 1] != '\n');
	return (in_state ? 0 : handle());
}

/*
 * Get just one char from the stream.
 */
char
get_char()
{
	char c;

	if (unget) {
		c = unget;
		unget = 0;
	} else
		read(sockfd, &c, 1);
	return c;
}

/*
 * Get an integer encoded in ASCII from the stream.
 */
int
get_int()
{
	int ret = 0;
	char c;

	/* Skip leading spaces */
	do
		c = get_char();
	while (c == ' ');

	while (isdigit(c)) {
		ret *= 10;
		ret += (c - 48);
		c = get_char();
	}
	unget = c;
	return ret;
}

void
get_eat(char c)
{
	char ch;

	do
		ch = get_char();
	while (c != ch);
}

/*
 * Send a char out to the server.
 */
void
put_char(char c)
{
	write(sockfd, &c, 1);
}

/*
 * Send a (Hollerith) string out to the server.
 */
void
put_string(char *str)
{
	char *buf;
	int len = strlen(str), totlen;

	totlen = asprintf(&buf, " %dH%s ", len, str);
	write(sockfd, buf, totlen);
}

void
get_accept(char ch)
{
	char c;

	/* Skip leading spaces */
	do
		c = get_char();
	while (c == ' ');
	prerr(c, ch);
}

char *
get_string()
{
	int len, i;
	char *c = "";

	len = get_int();
	get_accept('H');
	if (len) {
		c = calloc(len + 1, 1);
		for (i = 0; i < len; i++)
			c[i] = get_char();
	}
	return c;
}

void
prerr(char c, char ch)
{
	if (c != ch)
		errx(55, "Protocol error: got char %d expected %d", c, ch);
}
