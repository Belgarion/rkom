
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

#include "exported.h"
#include "rkom.h"
#include "set.h"

int	main(int, char **);

static int lasttime;

char *p_next_conf = "(Gå till) nästa möte";
char *p_next_text = "(Läsa) nästa inlägg";
char *p_see_time  = "(Se) tiden";
char *prompt;

int
main(int argc, char *argv[])
{
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
		gettimeofday(&tp, 0);
		if (tp.tv_sec - lasttime > 30) {
			rkom_alive();
			lasttime = tp.tv_sec;
		}
	}
	return 0;
}
