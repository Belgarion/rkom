
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#include "rkom_proto.h"
#include "exported.h"
#include "rkom.h"
#include "next.h"
#include "set.h"

int	main(int, char **);

static int lasttime;
static void sigio(int);
static int async_collect(void);

char *p_next_conf = "(Gå till) nästa möte";
char *p_next_text = "(Läsa) nästa inlägg";
char *p_see_time  = "(Se) tiden";
char *p_next_comment = "(Läsa) nästa kommentar";
char *prompt;

int
main(int argc, char *argv[])
{
	struct pollfd pfd[1];
	struct timeval tp;
	int ch, noprompt;
	char *server, *uname, *confile;

	confile = 0;
	prompt = p_see_time;
	while ((ch = getopt(argc, argv, "f:")) != -1)
		switch (ch) {
		case 'f':
			confile = optarg;
			break;

		default:
			(void)fprintf(stderr,
			    "usage: %s [-f file] [server]\n", argv[0]);
			exit(1);
		}
	argv += optind;
	argc -= optind;

	signal(SIGIO, sigio);

	if (argc != 1) {
		if ((server = getenv("KOMSERVER")) == 0)
			server = "kom.ludd.luth.se";
	} else
		server = argv[0];

	parsefile(confile);

	/* Attach to the KOM server */
	if ((uname = getenv("USER")) == NULL)
		uname = "amnesia";
	if (rkom_connect(server, NULL, uname))
		err(1, "misslyckades koppla upp...");

	pfd[0].fd = 0;
	pfd[0].events = POLLIN|POLLPRI;
	noprompt = 0;

	for (;;) {
		int rv;

		if (noprompt)
			noprompt = 0;
		else
			printf("\n%s - ", prompt);
		fflush(stdout);

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
			kbd_input(pfd[0].fd);
			gettimeofday(&tp, 0);
			if (tp.tv_sec - lasttime > 30) {
				rk_alive(0);
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
}

int
async_collect()
{
	struct rk_async *ra;
	char *hej;
	int retval = 1;

	while (1) {
		ra = rk_async(0);
		switch (ra->ra_type) {
		case 0:
			free(ra);
			return retval;

		case 15: /* New text created */
			hej = prompt;
			next_prompt();
			if (prompt != hej)
				retval = 0;
			break;

		default:
			printf("Ohanterat async %d\n", ra->ra_type);
			break;
		}
		free(ra);
	}
}
