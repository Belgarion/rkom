/* $Id: rkom.c,v 1.39 2001/11/25 20:07:48 jens Exp $ */

#ifdef SOLARIS
#undef _XPG4_2
#endif
#include <sys/types.h>
#include <sys/socket.h>
#ifdef POLL_EMUL
#include "poll_emul.h"
#else
#include <sys/poll.h>
#endif
#include <sys/time.h>
#ifndef SUNOS4
#include <sys/ioctl.h>
#endif

#include <netinet/in.h>

#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#if !defined(AIX)
#include <termcap.h>
#else
#endif
#include <errno.h>
#include <err.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#include "rkomsupport.h"
#include "rkom_proto.h"
#include "rkom.h"
#include "next.h"
#include "set.h"
#include "parse.h"
#include "rhistedit.h"

#define MAX_LINE	1024

int	main(int, char **);


static int lasttime;
static void sigio(int);
static void sigwinch(int);
static int async_collect(void);

static void setup_tty(int);
static void restore_tty(void);

static struct termios old_termios;

char *p_next_conf = "(Gå till) nästa möte";
char *p_next_text = "(Läsa) nästa inlägg";
char *p_see_time  = "(Se) tiden";
char *p_next_comment = "(Läsa) nästa kommentar";
char *prompt;
char *client_version = "ett.ett.beta";
int wrows, wcols, swascii;

#ifndef INFTIM
#define	INFTIM -1
#endif

static char *
prompt_fun(EditLine *el)
{
	static char		buf[MAX_LINE];

	sprintf(buf, "%s - ", prompt);
	return buf;
}

#if defined(SOLARIS) || defined(SUNOS4) || defined(AIX)
char * __progname;
#endif

int
main(int argc, char *argv[])
{
	HistEvent ev;
	History *hist;
	EditLine *el = NULL;
	const LineInfo  *lf;
	const char *str;
	char	buf[MAX_LINE];
	size_t	len;
	int	num;
	int	nullar = 0;
	struct pollfd	pfd[1];
	struct timeval	tp;
	int ch, noprompt;
	char *server, *uname, *confile;
	struct rk_server *rs;

#if defined(SOLARIS) || defined(SUNOS4)
	__progname  = strrchr(argv[0], '/');
	if (__progname == NULL)
		__progname = argv[0];
	else
		__progname++;
#endif
	confile = 0;
	prompt = p_see_time;
	while ((ch = getopt(argc, argv, "sf:")) != -1)
		switch (ch) {
		case 's':
			swascii++;
			break;

		case 'f':
			confile = optarg;
			break;

		default:
			(void)fprintf(stderr,
			    "usage: %s [-s] [-f file] [server]\n", argv[0]);
			exit(1);
		}
	argv += optind;
	argc -= optind;

	signal(SIGIO, sigio);
	signal(SIGWINCH, sigwinch);

	sigwinch(0);

	if (argc != 1) {
		if ((server = getenv("KOMSERVER")) == 0)
			server = "kom.ludd.luth.se";
	} else
		server = argv[0];

	parsefile(confile);

	/* Attach to the KOM server */
	if ((uname = getenv("USER")) == NULL)
		uname = "amnesia";
	if (rkom_fork())
		err(1, "kunde inte starta backend");

	rs = rk_connect(server, "", uname, client_version);
	if (rs->rs_retval)
		errx(1, "misslyckades koppla upp, error %d", rs->rs_retval);

	rprintf("Välkommen till raggkom kopplad till server %s.\n", server);
	rprintf("Servern kör %s, version %s. Protokollversion %d.\n",
	    rs->rs_servtype, rs->rs_version, rs->rs_proto);
	if (rs->rs_proto < 10) {
		rprintf("\nVARNING: Protokollversionen bör vara minst 10.\n");
		rprintf("VARNING: Vissa saker kan ofungera.\n");
	}
	free(rs);

	pfd[0].fd = 0;
	pfd[0].events = POLLIN|POLLPRI;
	noprompt = 0;

	setup_tty(1);
	hist = history_init();
	history(hist, &ev, H_SETSIZE, 200);
	el = el_init("rkom", stdin, stdout, stderr);
	el_set(el, EL_EDITOR, "emacs");	/* emacs binding */
	el_set(el, EL_PROMPT, prompt_fun);
	el_set(el, EL_HIST, history, hist);
	el_set(el, EL_TERMINAL, "vt100");

	for (;;) {
		int rv;

		if (noprompt)
			noprompt = 0;
		else
			rprintf("\n%s - ", prompt);
		fflush(stdout);

		setup_tty(0);
		rv = poll(pfd, 1, INFTIM);
		if (rv == 0)
			continue;
		if (rv < 0) {
			if (errno != EINTR)
				warn("poll");
			noprompt = async_collect();
			continue;
		}
		if (pfd[0].revents & (POLLIN|POLLPRI)) {
			/*
			 * Go to the beginning of the line to allow libedit
			 * to start
			 * from column 0 and overwrite the current prompt.
			 */
			outlines = 0;
			rprintf("%c", '\r');	
			if (el_gets(el, &num) == NULL) {
				if (nullar > 20)
					exit(0);
				nullar++;
				continue;
			}
			nullar = 0;
			lf = el_line(el);
			strncpy(buf, lf->buffer, MAX_LINE);
			buf[MAX_LINE-1] = 0;
			len = strlen(buf);
			while((len > 0 && buf[len - 1] == '\n')
#if defined(__FreeBSD__) /* isspace() is broken for non-ascii on NetBSD */
			    || isspace(buf[len-1])
#else
				|| buf[len - 1] == ' '
#endif
			    ) {
				buf[len - 1] = '\0';
				len--;
			}
			str = buf;
			exec_cmd(str);
			discard = 0;
#if !defined(__FreeBSD__)
			if (len > 1)
				history(hist, &ev, H_ENTER, buf);
#endif

			gettimeofday(&tp, 0);
			if (tp.tv_sec - lasttime > 30) {
				rk_alive();
				lasttime = tp.tv_sec;
			}
		}
		async_collect();
	}
	return 0;
}

void
sigio(int arg)
{
	signal(SIGIO, sigio);
}

void
sigwinch(int arg)
{
	struct winsize ws;

	ioctl(1, TIOCGWINSZ, (caddr_t)&ws);
	wrows = ws.ws_row;
	wcols = ws.ws_col;
}

int
async_collect()
{
	struct rk_async *ra;
	char *hej;
	int retval = 1;

	while (1) {
		ra = rk_async();
		switch (ra->ra_type) {
		case 0:
			free(ra);
			return retval;

		case 9:
		case 13: {
			struct rk_conference *sender;

			if (iseql("presence-messages", "1")) {
				sender = rk_confinfo(ra->ra_pers);
				rprintf("\n%s har just loggat %s.",
				    sender->rc_name,
				    (ra->ra_type == 13 ? "ut" : "in"));
				free(sender);
				retval = 0;
			}
		}
		break;

		case 5: 
			if (iseql("presence-messages", "1")) {
				rprintf("\n%s bytte just namn till %s.",
				    ra->ra_message, ra->ra_message2);
				retval = 0;
			}
		break;

		case 12: {
			struct rk_conference *sender, *rcpt;
			struct rk_time *tm;

			sender = rk_confinfo(ra->ra_pers);
rprintf("\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
			if (ra->ra_conf == 0)
				rprintf("Allmänt meddelande från %s",
				    sender->rc_name);
			else if (ra->ra_conf == myuid)
				rprintf("Personligt meddelande från %s",
				    sender->rc_name);
			else {
				rcpt = rk_confinfo(ra->ra_conf);
				rprintf("Meddelande till %s från %s",
				    rcpt->rc_name, sender->rc_name);
				free(rcpt);
			}
			tm = rk_time();
			rprintf(" (%s):\n\n", get_date_string(tm));
			free(tm);
			rprintf("%s\n", ra->ra_message);
rprintf("----------------------------------------------------------------\n");
			retval = 0;
		}
		break;

		case 15: /* New text created */
			if (ra->ra_pers == myuid &&
			    iseql("created-texts-are-read", "1")) {
				mark_read(ra->ra_text);
				break;
			}
			hej = prompt;
			if (prompt != PROMPT_NEXT_COMMENT)
				next_prompt();
			if (prompt != hej)
				retval = 0;
			break;

		case 8:
		case 14:
		case 18:
			break;

		default:
			rprintf("Ohanterat async %d\n", ra->ra_type);
			break;
		}
		free(ra);
	}
}

#ifdef SUNOS4
void on_exit(void (*function)(void));	/* XXX */
#define	atexit	on_exit
#endif

static void
setup_tty(int save_old)
{
	struct termios t;

	if (tcgetattr(0, &t) < 0)
		err(1, "tcgetattr");

	if (save_old) {
		old_termios = t;
		atexit(restore_tty);
	}

	t.c_lflag &= ~(ECHO | ICANON);
	t.c_cc[VMIN] = 1;	/* 1 byte at a time, no timer */
	t.c_cc[VTIME] = 0;

	if (tcsetattr(0, TCSAFLUSH, &t) < 0)
		err(1, "tcsetattr");
}

static void
restore_tty(void)
{
	tcsetattr(0, TCSAFLUSH, &old_termios);
}
