
#include <sys/time.h>
#ifdef POLL_EMUL
#include "poll_emul.h"
#else
#include <sys/poll.h>
#endif

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <err.h>

#include "rkomsupport.h"
#include "rkom_proto.h"
#include "backend.h"
#include "rtype.h"

#ifndef INFTIM
#define	INFTIM -1
#endif

static int reqnr;
static int wait_for_reply_no;
int sockfd;
static int unget;
static int level;

struct callback {
	int msgid;
	int arg;
	struct callback *next;
	void (*func)(int, int);
};

static struct callback *cpole;

/*
 * Main loop. Running normally on level 1, waiting for replies on 
 * level 2. Handles async requests only on level 1.
 */
int
rkom_loop()
{
	struct pollfd pfd[2];
	int erra = 0;

	/*
	 * Set up the poll descriptors.
	 */
	level++;
	pfd[0].fd = readfd;
	pfd[0].events = (level < 2 ? POLLIN|POLLPRI : 0);
	pfd[1].fd = sockfd;
	pfd[1].events = POLLIN|POLLPRI;

	/*
	 * Ok, everything seems OK. Get into the poll loop and wait
	 * for things to happen.
	 */
	for (;;) {
		int rv;
	
		if (level == 1)
			async_handle();
		/* Wait for something to happen */
		pfd[0].revents = pfd[1].revents = 0;
		rv = poll(pfd, 2, INFTIM);
		if (rv == 0)
			continue;
		if (rv < 0) {
			if (errno != EINTR)
				warn("poll");
			continue;
		}
		if (pfd[0].revents & (POLLIN|POLLPRI))
			spc_process_request();
		if (pfd[1].revents & (POLLIN|POLLPRI)) {
			int i;
			char c;

			/* Check if there are anything available */
			if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
				err(1, "fcntl");
			if (read(sockfd, &c, 1) != 1) {
				err(1, "read sockfd: säg till ragge");
				continue;
			}
			unget = c;
			if (fcntl(sockfd, F_SETFL, 0) == -1)
				err(1, "fcntl2");

			c = get_char();

			switch (c) {
			case ':':
				async(level);
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
				erra = -1;
				unget = c;
				/* FALLTHROUGH */
			case '=':
				i = get_int();
				if (wait_for_reply_no == i) {
					wait_for_reply_no = 0;
					level--;
					return erra;
				} else {
					struct callback *nc = 0, *cc = cpole;
					while (cc) {
						if (cc->msgid == i) {
							(*cc->func)
							    (erra, cc->arg);
							if (cc == cpole) {
								cpole =
								    cc->next;
							} else {
								nc->next =
								    cc->next;
							}
							free(cc);
							erra = 0;
							break;
						}
						nc = cc;
						cc = cc->next;
					}
					if (cc == 0) {
						printf("Okänd reply ");
						while (c != '\n') {
							c = get_char();
							putchar(c);
						}
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
	}
}

int
send_reply(char *msg)
{
	static int in_state;
	char buf[12];

	if (in_state == 0) {
		wait_for_reply_no = reqnr;
		sprintf(buf, "%d ", reqnr++);
		write(sockfd, buf, strlen(buf));
	}
	write(sockfd, msg, strlen(msg));
	in_state = (msg[strlen(msg) - 1] != '\n');
	return (in_state ? 0 : rkom_loop());
}

void
send_callback(char *msg, int arg, void (*func)(int, int))
{
	struct callback *c;
	char buf[12];

	sprintf(buf, "%d ", reqnr);
	write(sockfd, buf, strlen(buf));
	write(sockfd, msg, strlen(msg));
	c = malloc(sizeof(struct callback));
	c->func = func;
	c->arg = arg;
	c->msgid = reqnr;
	c->next = cpole;
	cpole = c;
	reqnr++;
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

	while (isdigit((int)c)) {
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

	buf = malloc(len+20);
#ifdef SUNOS4
	sprintf(buf, " %dH%s ", len, str);
	totlen = strlen(buf);
#else
	totlen = sprintf(buf, " %dH%s ", len, str);
#endif
	write(sockfd, buf, totlen);
}

static void
prerr(char c, char ch)
{
	if (c != ch)
		errx(55, "Protocol error: got char %d expected %d", c, ch);
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

