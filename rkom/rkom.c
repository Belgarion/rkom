
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
static void async_collect(void);

char *p_next_conf = "(G� till) n�sta m�te";
char *p_next_text = "(L�sa) n�sta inl�gg";
char *p_see_time  = "(Se) tiden";
char *p_next_comment = "(L�sa) n�sta kommentar";
char *prompt;
int fetchstat;

int
main(int argc, char *argv[])
{
	struct sigaction sa;
	struct timeval tp;
	int ch;
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

	bzero(&sa, sizeof(sa));
	sa.sa_handler = sigio;
	sigaction(SIGIO, &sa, NULL);

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

	for (;;) {
		printf("\n%s - ", prompt);
		fflush(stdout);
		kbd_input(0);
		if (fetchstat) {
			fetchstat = 0;
			async_collect();
		}
		gettimeofday(&tp, 0);
		if (tp.tv_sec - lasttime > 30) {
			rk_alive(0);
			lasttime = tp.tv_sec;
		}
	}
	return 0;
}

void
sigio(int arg)
{
	fetchstat = 1;
}

void
async_collect()
{
	struct rk_async *ra;

	while (1) {
		ra = rk_async(0);
		switch (ra->ra_type) {
		case 0:
			free(ra);
			return;

		case 15: /* New text created */
			next_prompt();
			break;

		default:
			printf("Ohanterat async %d\n", ra->ra_type);
			break;
		}
		free(ra);
	}
}
