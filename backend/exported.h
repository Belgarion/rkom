/*	$Id: exported.h,v 1.1 2000/09/26 18:48:00 ragge Exp $	*/
/*
 * Exported functions from the rkom frontend/backend communication system.
 *
 * NOTE:
 *	Nothing that is malloc'ed in the comm system do ever need to
 *	be free'd in the frontend! That is handled in the comm system.
 *
 *	At the end of this file is descriptions of the different messages
 *	that must be understood by the frontend.
 */

/*
 * Forks the backend process. This call should be the first call.
 * Do the initial connection to the lyskom server.
 * Only parameters that do not change are sent here.
 * Parameters are:
 *	server		Lyskom server hostname to connect to.
 *	frontend	String with front-end name identifier.
 *	os_username	The host username that runs the client, not to
 *			confuse with the Lyskom username.
 *
 * Return values:
 *	0		Succeeded.
 *	-1		Something failed (more useable msg will be added)
 */
int	rkom_connect(char *server, char *frontend, char *os_username);

/*
 * Matches a Lyskom username with the on-server usernames.
 * Parameters are:
 *	user		A string with a username to match.
 *	matched		Pointer to a pointer to an array of char pointers.
 *
 * Return values:
 *	0-x		Returns number of usernames matched.
 *
 *	The "matched" array will also be filled in with the usernames
 *	that matches the given user.
 */
int	rkom_matchuser(char *user, char **matched[]);

/*
 * Login user on the current system.
 * Parameters are:
 *	user		A Lyskom username.
 *	passwd		The corresponding password.
 *
 * Return values:
 *	0		Login succeeded
 *	-1		Login failed
 */
int	rkom_login(char *user, char *passwd);

/*
 * Log out the user from the current system.
 * This call never fails.
 */
void	rkom_logout(void);

/*
 * Ask for all conferences on the system.
 * Parameter is:
 *	confs		Pointer to a pointer to an array of char pointers.
 *
 * Return values:
 *	0-x		Number of returned conferences.
 *
 *	The confs array is filled in with conference info, format of these
 *	strings are described in "CONFERENCE STRINGS".
 */
int	rkom_conf(char **confs[]);

/*
 * Ask for all conferences with unread messages.
 * Parameter is:
 *	confs		Pointer to a pointer to an array of char pointers.
 *
 * Return values:
 *	0-x		Number of returned conferences.
 *
 *	The confs array is filled in with conference info, format of these
 *	strings are described in "CONFERENCE STRINGS".
 */
int	rkom_unread_conf(char **confs[]);

/*
 * Informs the server that the user is working.
 * This call never fails.
 */
void	rkom_alive(void);





/*
 * ASYNC MESSAGES:
 *	TBD...
 */
/*
 * CONFERENCE STRINGS:
 *	TBD...
 */
