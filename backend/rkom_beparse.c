
#include <sys/types.h>
#include <sys/uio.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "backend.h"
#include "exported.h"
#include "protocol.h"

int myuid;

static	void a2numwrapper(char *);
static	void login(char *);
static	void unreadconfs(int *);
static	void confinfo(int *);
static	void persinfo(int *);
static	void membership(void *);
static	void who(void *);

void
rkom_beparse(int cmd, void *args, int arglen)
{
	char *buf;
	int i;

	switch (cmd) {
	case TIME: {
		struct tm tm;

		send_reply("35\n");
		read_in_time(&tm);
		get_accept('\n');
		bgsend(0, sizeof(struct tm), &tm);
		break;
	}

	case ALIVE:
		send_reply("82\n");
		get_accept('\n');
		bgsend(0, 0, 0);
		break;

	case MATCHUSER:
		a2numwrapper(args);
		break;

	case LOGIN:
		login(args);
		break;

	case WHATIDO:
		buf = alloca(strlen(args) + 30);
		sprintf(buf, "4 %ldH%s\n", (long)strlen(args), (char *)args);
		i = send_reply(buf);
		get_eat('\n');
		bgsend(i, 0, 0);
		break;

	case UNREADCONF:
		unreadconfs(args);
		break;

	case CONFINFO:
		confinfo(args);
		break;

	case MEMBERSHIP:
		membership(args);
		break;

	case WHO:
		who(args);
		break;

	case PERSINFO:
		persinfo(args);
		break;

	default:
		printf("rkom_beparse: cmd %d arglen %d\n", cmd, arglen);
		bgsend(0, 0, 0);
		break;
	}
}

static void
a2numwrapper(char *name)
{
	struct iovec *iov;
	char *buf, *ptr;
	int antal, junk, i, len, p, k;

	bcopy(&name[strlen(name) + 1], &i, sizeof(int));
	k = (i & MATCHCONF_CONF) == MATCHCONF_CONF;
	p = (i & MATCHCONF_PERSON) == MATCHCONF_PERSON;
	buf = alloca(strlen(name) + 30);
	sprintf(buf, "76 %ldH%s %d %d\n", (long)strlen(name), name, p, k);
	free(name);
	send_reply(buf);

	antal = get_int();
	if (antal == 0) {
		get_eat('\n');
		bgsend(0, 0, 0);
		return;
	}
	iov = alloca(sizeof(struct iovec) * antal);
	get_accept('{');
	for (i = 0; i < antal; i++) {
		buf = get_string();
		len = strlen(buf) + 2 * sizeof(int) + 1;
		iov[i].iov_base = alloca(len);
		iov[i].iov_len = len;
		ptr = iov[i].iov_base;
		strcpy(ptr, buf);
		ptr += strlen(buf) + 1;
		junk = get_int();
		bcopy(&junk, ptr, sizeof(int));
		ptr += sizeof(int);
		junk = get_int();
		bcopy(&junk, ptr, sizeof(int));
		free(buf);
	}
	get_accept('}');
	get_accept('\n');
	bgsendv(0, antal, iov);
}

static void
login(char *passwd)
{
	char *buf;
	int uid, i;

	bcopy(&passwd[strlen(passwd) + 1], &uid, sizeof(int));

	buf = alloca(strlen(passwd) + 30);
	sprintf(buf, "62 %d %ldH%s 0\n", uid, (long)strlen(passwd), passwd);
	if (send_reply(buf)) {
		i = get_int();
	} else {
		myuid = uid;
		i = 0;
	}
	bgsend(i, 0, 0);
}

static void     
unreadconfs(int *uconf)
{
	int *p, i, nconfs, size;
	char buf[15];

	sprintf(buf, "52 %d\n", *uconf);
	i = send_reply(buf);
	if ((i = send_reply(buf))) {
		get_eat('\n');
		bgsend(i, 0, 0);
		return;
	}
	nconfs = get_int();
	if (nconfs == 0) {
		get_eat('\n');
		bgsend(0, 0, 0);
		return;
	}
	size = nconfs * sizeof(int);
	p = alloca(size);
	get_accept('{');
	for (i = 0; i < nconfs; i++)
		p[i] = get_int();
	get_accept('}');
	get_accept('\n');
	bgsend(0, size, p);
}


static void
confinfo(int *confno)
{
	struct iovec iov[2];
	struct conference *conf;
	int ret;

	ret = get_conf_stat(*confno, &conf);
	if (ret) {
		bgsend(ret, 0, 0);
		return;
	}
	iov[0].iov_base = conf;
	iov[0].iov_len = sizeof(struct conference);
	iov[1].iov_base = conf->name;
	iov[1].iov_len = strlen(conf->name) + 1;
	bgsendv(0, 2, iov);
}

static void
persinfo(int *persno)
{
	struct iovec iov[2];
	struct person *person; 
	int ret;

	ret = get_pers_stat(*persno, &person);
	if (ret) {
		bgsend(ret, 0, 0);
		return;
	}
	iov[0].iov_base = person;
	iov[0].iov_len = sizeof(struct person);
	iov[1].iov_base = person->username;
	iov[1].iov_len = strlen(person->username) + 1;
	bgsendv(0, 2, iov);
}               

static void
membership(void *args)
{
	struct iovec iov[2];
	struct membership *m;
	int uid, mid, ret;

	bcopy(args, &uid, sizeof(int));
	bcopy((char *)(args) + sizeof(int), &mid, sizeof(int));
	ret = get_membership(uid, mid, &m);
	if (ret) {
		bgsend(ret, 0, 0);
		return;
	}
	iov[0].iov_base = m;
	iov[0].iov_len = sizeof(struct membership);
	iov[1].iov_base = m->read_texts;
	iov[1].iov_len = m->nread_texts * sizeof(int);
	bgsendv(0, 2, iov);
}

void
who(void *args)
{
	struct dynamic_session_info *pp;
	struct iovec *iov;
	char buf[50];
	int secs, flags, antal, i;

	bcopy(args, &secs, sizeof(int));
	bcopy((char *)(args) + sizeof(int), &flags, sizeof(int));

	sprintf(buf, "83 %d %d %d\n", (flags & WHO_VISIBLE) != 0, 
	    (flags & WHO_INVISIBLE) != 0, secs);
	send_reply(buf);

	antal = get_int();
	if (antal == 0) {
		struct iovec i;
		bzero(&i, sizeof(struct iovec));
		bgsendv(0, 1, &i);
		get_eat('\n');
		return;
	}
	pp = alloca(sizeof(struct dynamic_session_info) * (antal + 1));
	bzero(pp, sizeof(struct dynamic_session_info) * (antal + 1));
	get_accept('{');
	for (i = 0; i < antal; i++) {
		pp[i].session = get_int();
		pp[i].person = get_int();
		pp[i].conf = get_int();
		pp[i].idletime = get_int();
		pp[i].flags = get_int();
		pp[i].doing = get_string();
	}
	get_accept('}');
	get_accept('\n');
	iov = alloca(sizeof(struct iovec) * (antal + 1) * 2);
	iov[0].iov_base = pp;
	iov[0].iov_len = sizeof(struct dynamic_session_info) * (antal + 1);
	for (i = 0; i < antal; i++) {
		iov[i + 1].iov_base = pp[i].doing;
		iov[i + 1].iov_len = strlen(pp[i].doing) + 1;
	}
	bgsendv(0, antal + 1, iov);
}
