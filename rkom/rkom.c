/* $Id: rkom.c,v 1.57 2003/10/01 16:23:36 ragge Exp $ */

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

#include "rkomsupport.h"
#include "rkom.h"
#include "next.h"
#include "set.h"
#include "parse.h"
#include "rhistedit.h"
#include "backend.h"
#include "rtype.h"

#define MAX_LINE	1024

int	main(int, char **);


static int lasttime;
static void sigwinch(int);

static void setup_tty(int);
static void restore_tty(void);

static struct termios old_termios;

EditLine *main_el;

char *p_next_conf = "(Gå till) nästa möte";
char *p_next_text = "(Läsa) nästa inlägg";
char *p_see_time  = "(Se) tiden";
char *p_next_comment = "(Läsa) nästa kommentar";
char *p_next_marked = "(Återse) nästa markerade";
char *prompt, *server;
char *client_version = "ett.två.alfa";
int wrows, wcols, swascii;
int noprompt;
HistEvent ev;
History *hist;

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
	int ch;
	char *uname, *confile, *termtype;
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
	if ((rs = rkom_connect(server, "", uname, client_version)) == NULL)
		errx(1, "misslyckades koppla upp, error %d", komerr);

	rprintf("Välkommen till raggkom kopplad till server %s.\n", server);
	rprintf("Servern kör %s, version %s. Protokollversion %d.\n",
	    rs->rs_servtype, rs->rs_version, rs->rs_proto);
	if (rs->rs_proto < 10) {
		rprintf("\nVARNING: Protokollversionen bör vara minst 10.\n");
		rprintf("VARNING: Vissa saker kan ofungera.\n");
	}

	if ((termtype = getenv("TERM")) == NULL)
		termtype = "vt100";
	hist = history_init();
	history(hist, &ev, H_SETSIZE, 200);
	main_el = el_init("rkom", stdin, stdout, stderr);
	el_set(main_el, EL_EDITOR, getval("editor-mode"));
	el_set(main_el, EL_PROMPT, prompt_fun);
	el_set(main_el, EL_HIST, history, hist);
	el_set(main_el, EL_TERMINAL, termtype);

	rprintf("\n%s - ", prompt);
	async_collect();
	fflush(stdout);
	setup_tty(1);
	rkom_loop();
	return 0;
}

void
rkom_command()
{
	const char *str;
	struct timeval	tp;
	const LineInfo  *lf;
	unsigned char	buf[MAX_LINE];
	int num;
	size_t	len;

	/*
	 * Go to the beginning of the line to allow libedit
	 * to start
	 * from column 0 and overwrite the current prompt.
	 */
	outlines = 0;
	rprintf("%c", '\r');	
	if (el_gets(main_el, &num, 1) == NULL)
		return;

	lf = el_line(main_el);
	strncpy(buf, lf->buffer, MAX_LINE);
	buf[MAX_LINE-1] = 0;
	len = strlen(buf);
	while((len > 0 && buf[len - 1] == '\n') || isspace(buf[len-1])) {
		buf[len - 1] = '\0';
		len--;
	}
	str = buf;
	exec_cmd(str);
	discard = 0;
	if (len > 1)
		history(hist, &ev, H_ENTER, buf);

	gettimeofday(&tp, 0);
	if (tp.tv_sec - lasttime > 30) {
		rk_alive();
		lasttime = tp.tv_sec;
	}

	rprintf("\n%s - ", prompt);
	fflush(stdout);
	setup_tty(1);
}

void
sigwinch(int arg)
{
	struct winsize ws;

	ioctl(1, TIOCGWINSZ, (caddr_t)&ws);
	wrows = ws.ws_row;
	wcols = ws.ws_col;
}

void
async_collect()
{
	struct rk_async *ra;
	char *hej;
	int retval = 1;

	while (1) {
		ra = rk_async();
		switch (ra->ra_type) {
		case 0:
			if (retval == 0)
				rprintf("\n%s - ", prompt);
			fflush(stdout);
			return;

		case 9:
		case 13: {
			struct rk_conference *sender;

			if (iseql("presence-messages", "1")) {
				if ((sender = rk_confinfo(ra->ra_pers)) == NULL)
					hej = "John Doe";
				else
					hej = sender->rc_name;
				rprintf("\n%s har just loggat %s.", hej,
				    (ra->ra_type == 13 ? "ut" : "in"));
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
			char *name;

			if (isneq("only-private-messages", "0") &&
			    ra->ra_conf != myuid)
				break;
			if ((sender = rk_confinfo(ra->ra_pers)) == NULL)
				hej = "John Doe";
			else
				hej = sender->rc_name;
rprintf("\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
			if (ra->ra_conf == 0) {
				rprintf("Allmänt meddelande från %s", hej);
			} else if (ra->ra_conf == myuid) {
				rprintf("Personligt meddelande från %s", hej);
			} else {
				if ((rcpt = rk_confinfo(ra->ra_conf)) == NULL)
					name = "Jane Doe";
				else
					name = rcpt->rc_name;
				rprintf("Meddelande till %s från %s",
				    name, hej);
			}
			tm = rk_time();
			rprintf(" (%s):\n\n", get_date_string(tm));
			rprintf("%s\n", ra->ra_message);
rprintf("----------------------------------------------------------------\n");
			if (isneq("beep-on-private-messages", "0"))
				printf("%c", 7);
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
			if (prompt != hej) {
				if (iseql("disable-beep-on-new-messages", "0"))
					putchar(007); /* Signal new text */
				retval = 0;
			}
			break;

		case 8:
		case 14:
		case 18:
			break;

		default:
			rprintf("Ohanterat async %d\n", ra->ra_type);
			break;
		}
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

	if (tcsetattr(0, TCSANOW, &t) < 0)
		err(1, "tcsetattr");
}

static void
restore_tty(void)
{
	tcsetattr(0, TCSAFLUSH, &old_termios);
}
