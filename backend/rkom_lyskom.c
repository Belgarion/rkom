
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
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include "rkomsupport.h"
#include "backend.h"
#include "rtype.h"

#ifndef INFTIM
#define	INFTIM -1
#endif

static int reqnr;
static int wait_for_reply_no;
static int unget;
extern FILE *sfd;
extern int pfd;
static int curc, maxc;

struct callback {
	int msgid;
	int arg;
	struct callback *next;
	void (*func)(int, int);
};

static struct callback *cpole;

/*
 * Main loop.
 */
int
rkom_loop(int which)
{
	struct pollfd pofd[2];
	int erra = 0;

	/*
	 * Set up the poll descriptors.
	 */
	pofd[0].fd = 0;	/* stdin */
	pofd[0].events = (which & POLL_KEYBOARD ? POLLIN|POLLPRI : 0);
	pofd[1].fd = pfd;
	pofd[1].events = (which & POLL_NETWORK ? POLLIN|POLLPRI : 0);

	/*
	 * Ok, everything seems OK. Get into the poll loop and wait
	 * for things to happen.
	 */
	for (;;) {
		int rv;
	
		if (which & POLL_ASYNCS)
			async_collect();
		/* Wait for something to happen */
		pofd[0].revents = pofd[1].revents = 0;
		if (curc != maxc)
			goto gotchar;
		rv = poll(pofd, 2, INFTIM);
		if (rv == 0)
			continue;
		if (rv < 0) {
			if (errno != EINTR)
				err(1, "poll: säg till ragge");
			continue;
		}
		if (pofd[0].revents & (POLLIN|POLLPRI)) {
			if (which & POLL_RET_KBD)
				return 0;
			rkom_command();
		}
		if (pofd[1].revents & (POLLIN|POLLPRI)) {
			int i;
			char c;

gotchar:		c = get_char();

			switch (c) {
			case ':':
				async(which & POLL_ASYNCS);
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

#ifdef __STDC__
int
send_reply(char *msg, ...)
#else
int
send_reply(msg, va_alist)
	char *msg;
	va_dcl
#endif
{
	static int in_state;
	va_list ap;

#ifdef __STDC__
	va_start(ap, msg);
#else
	va_start(ap);
#endif
	if (in_state == 0) {
		wait_for_reply_no = reqnr;
		fprintf(sfd, "%d ", reqnr++);
	}
	vfprintf(sfd, msg, ap);
	va_end(ap);
	in_state = (msg[strlen(msg) - 1] != '\n');
	return (in_state ? 0 : rkom_loop(POLL_NETWORK));
}

void
send_callback(char *msg, int arg, void (*func)(int, int))
{
	struct callback *c;

	fprintf(sfd, "%d %s", reqnr, msg);
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
	static char buffer[BUFSIZ];
	char c;

	if (unget) {
		c = unget;
		unget = 0;
	} else {
		if (curc == maxc) {
			maxc = read(pfd, buffer, BUFSIZ);
			curc = 0;
		}
		c = buffer[curc++];
	}
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

/*
 * Get a bitfield from the input stream.
 */
int
get_bitfield()
{
	int bf = 0, val = get_int();

	if (val >= 100000000)
		errx(1, "bitfield too large: %d", val);

	bf |= (val/10000000); val -= (10000000 * (bf & 1));
	bf <<= 1; bf |= (val/1000000); val -= (1000000 * (bf & 1));
	bf <<= 1; bf |= (val/100000); val -= (100000 * (bf & 1));
	bf <<= 1; bf |= (val/10000); val -= (10000 * (bf & 1));
	bf <<= 1; bf |= (val/1000); val -= (1000 * (bf & 1));
	bf <<= 1; bf |= (val/100); val -= (100 * (bf & 1));
	bf <<= 1; bf |= (val/10); val -= (10 * (bf & 1));
	bf <<= 1; bf |= val;
	return bf;
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
	fputc(c, sfd);
}

/*
 * Send a (Hollerith) string out to the server.
 */
void
put_string(char *str)
{

	fprintf(sfd, " %ldH%s ", (long)strlen(str), str);
}

static void
prerr(char c, char ch)
{
	if (c == ch)
		return;

	fprintf(stderr, "Protocol error: got char %d expected %d", c, ch);
	abort();
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

