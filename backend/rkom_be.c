

#include <sys/types.h>
#include <sys/uio.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "backend.h"

static int
sread(int fd, void *p, size_t n)
{
	int i = 0, j;
	char *c = p;

	while (i != n) {
		j = i;
		i += read(fd, c + i, n - i);
		if (j == i)
			exit(0); /* Read 0, pipe closed */
	}
	return n;
}
/*
 * A request sent consists of an int command followed by optional
 * arguments. A reply consists of an int which is the length on
 * the following string to be received.
 */
int
fgrw(int cmd, void *arg, int alglen, void **reply, int *replylen)
{
	int len, ret;
	void *svar = 0;

	write(writefd, &cmd, sizeof(int));
	write(writefd, &alglen, sizeof(int));
	if (alglen)
		write(writefd, arg, alglen);
	sread(readfd, &ret, sizeof(int));
	sread(readfd, &len, sizeof(int));
	if (len) {
		svar = malloc(len);
		sread(readfd, svar, len);
	}
	if (reply)
		*reply = svar;
	if (replylen)
		*replylen = len;
	return ret;
}

void
bgreceive()
{
	int cmd, len;
	void *svar = 0;

	sread(readfd, &cmd, sizeof(int));
	sread(readfd, &len, sizeof(int));
	if (len) {
		svar = malloc(len);
		sread(readfd, svar, len);
	}
	rkom_beparse(cmd, svar, len);
}

void
bgsend(int retval, int len, void *reply)
{
	write(writefd, &retval, sizeof(int));
	write(writefd, &len, sizeof(int));
	if (len)
		write(writefd, reply, len);
}

void
bgsendv(int retval, int elem, struct iovec *iov)
{
	int len, i;

	write(writefd, &retval, sizeof(int));
	for (len = i = 0; i < elem; i++)
		len += iov[i].iov_len;

	write(writefd, &len, sizeof(int));
	for (i = 0; i < elem; i++)
		write(writefd, iov[i].iov_base, iov[i].iov_len);
}
