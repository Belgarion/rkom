/* $Id: ckom.c,v 1.1 2000/10/15 11:59:39 jens Exp $ */

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <err.h>

#include <rkom_proto.h>
#include <exported.h>

#include "articles.h"
#include "scr.h"
#include "conf.h"

#define ENV_KOMSERVER	"KOMSERVER"
#define DEF_KOMSERVER	"kom.ludd.luth.se"

static const char *progname;

void usage __P((void));

int
main(int argc, char *argv[])
{
	struct rk_confinfo_retval *rcr;
	struct rk_unreadconfval *ruc;
	conft_t		*ct;
	char		*kom_server;
	char		*user_name, *pass_word;
	u_int32_t	myuid;

	progname = argv[0];
	if (argc == 1) {
		if ((kom_server = getenv(ENV_KOMSERVER)) == NULL)
			kom_server = DEF_KOMSERVER;
	} else if (argc == 2)
		kom_server = argv[1];
	else
		usage();
	/* initialize the screen */
	scr_start();

	/* connect to the kom server */
	if ((user_name = getenv("USER")) == NULL)
		user_name = "amnesia";
	printf("connecting to server\n");
	if (rkom_connect(kom_server, NULL, user_name) != 0)
		errx(1, "failed to connect to %s: %s", kom_server, strerror(errno));

	user_name = "testrkomjens";
	rcr = rk_matchconf(user_name, MATCHCONF_PERSON);
	if (rcr->rcr_ci.rcr_ci_len != 1)
		errx(1, "wrong rcr->rcr_ci.rcr_ci_len: %d", rcr->rcr_ci.rcr_ci_len);
	myuid = rcr->rcr_ci.rcr_ci_val[0].rc_conf_no;
	free(rcr);

	pass_word = "3hilmery";
	if (rk_login(myuid, pass_word) != 0)
		errx(1, "login failed");
	printf("logged in\n");

	if (rk_whatido("Kör curses-klienten (under utveckling)") != 0)
		errx(1, "rk_whatido failed");

	ruc = rk_unreadconf(myuid);
	if (ruc->ru_confs.ru_confs_len == 0)
		errx(0, "Inga olästa inlägg");
	ct = art_get_arts(myuid, ruc->ru_confs.ru_confs_val[0]);
	conference_menu(ct);

	/* shut down the screen */
	scr_cleanup();
	return 0;
}

void
usage(void)
{
	fprintf(stderr, "usage: %s [server]\n", progname);
	exit(1);
}
