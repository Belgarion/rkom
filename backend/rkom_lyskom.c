
#include <sys/time.h>
#include <sys/poll.h>

#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <err.h>

#include "backend.h"

static int reqnr;
static int handling;	/* Handling front-end request right now */
static int wait_for_reply_no;
int sockfd;
static int unget;
static int level;

/*
 * Main loop. Running normally on level 1, waiting for replies on 
 * level 2. Handles async requests only on level 1.
 */
int
rkom_loop()
{
	struct pollfd pfd[2];
	int err = 0;

	/*
	 * Set up the poll descriptors.
	 */
	pfd[0].fd = readfd;
	pfd[0].events = POLLIN|POLLPRI;
	pfd[1].fd = sockfd;
	pfd[1].events = POLLIN|POLLPRI;
	level++;

	/*
	 * Ok, everything seems OK. Get into the poll loop and wait
	 * for things to happen.
	 */
	for (;;) {
		int rv;
	
#ifdef notyet
		if (level == 0)
			async_handle();
#endif
		/* Wait for something to happen */
		rv = poll(pfd, 2, INFTIM);
		if (rv == 0)
			continue;
		if (rv < 0) {
			if (errno != EINTR)
				warn("poll");
			continue;
		}
		if (pfd[0].revents) {
			if (handling++)
				warn("front-end unwanted talk");
			bgreceive();
			handling = 0;
		}
		if (pfd[1].revents & (POLLIN|POLLPRI)) {
			int i;
			char c = get_char();

			switch (c) {
			case ':':
#ifdef notyet
				async(level);
				break;
#endif

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
				err = -1;
				unget = c;
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

