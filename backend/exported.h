/*	$Id: exported.h,v 1.3 2000/10/07 10:38:46 ragge Exp $	*/
/*
 * Exported functions from the rkom frontend/backend communication system.
 *
 * NOTE:
 *	At the end of this file is descriptions of the different messages
 *	that must be understood by the frontend.
 */

/* Some forward declarations */
struct tm;

/* Structures used for communication */
/* Person/conference information when matching a string */
struct confinfo {
	char *name;	/* Conference name */
	int type;	/* flags set in conference */
	int conf_no;	/* Number of this conference */
};

/* aux-info in conference struct */
struct aux_item {
	int		aux_no;
	int		tag;
	int		creator;
	struct tm	created_at;
	int		flags;
	int		inherit_limit;
	char *		data;
};

/*
 * Type of membership in a conference.
 * This is bits stored in an int.
 */
#define MT_INVITATION	0x80
#define MT_PASSIVE	0x40
#define MT_SECRET	0x20

struct membership {
	int		position;
	struct tm	last_time_read;
	int		conference;
	int		priority;
	int		last_text_read;
	int		nread_texts; /* Size of the following array */
	int *		read_texts;
	int		added_by;
	struct tm	added_at;
	int		type;
};


/* Extended-Conf-Type described as bits in an int */
#define ECT_RD_PROT	0x80
#define ECT_ORIGINAL	0x40
#define ECT_SECRET	0x20
#define ECT_LETTERBOX	0x10
#define ECT_ALLOW_ANON	0x08
#define ECT_FORBID_SECR 0x04


/* Full information about a conference */
struct conference {
	char *		name;
	int		type;
	struct tm	creation_time;
	struct tm	last_written;
	int		creator;
	int		presentation;
	int		supervisor;
	int		permitted_submitters;
	int		super_conf;
	int		msg_of_day;
	int		nice;
	int		keep_commented;
	int		no_of_members;
	int		first_local_no;
	int		no_of_texts;
	int		expire;
	struct aux_item *aux_items;
};

/* Information about logged-on persons */
struct dynamic_session_info {
	int	session;
	int	person;
	int	conf;
	int	idletime;
	int	flags;
	char	*doing;
};

/* statistics for a person */
struct person {
	char 		*username;
	int		privileges;
	int		flags;
	struct tm	last_login;
	int		user_area;
	int		total_time_present;
	int		sessions;
	int		created_lines;
	int		created_bytes;
	int		read_texts;
	int		no_of_text_fetches;
	int		created_persons;
	int		created_confs;
	int		first_created_local_no;
	int		no_of_created_texts;
	int		no_of_marks;
	int		no_of_confs;
};

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
 * Matches a Lyskom conference with the on-server conference names.
 * Parameters are:
 *	conf		A string with a conference name to match.
 *	flags		Type of reply, see below.
 *	matched		Pointer to a pointer to a struct confinfo.
 *
 * Return values:
 *	0-x		Returns number of usernames matched.
 *
 *	The "matched" array will also be filled in with the conference names
 *	that matches the given conference.
 *
 *	The pointers *matched[0].name and *matched must be freed by the 
 *	calling routine, in that order.
 */
#define MATCHCONF_PERSON	1
#define MATCHCONF_CONF		2
int	rkom_matchconf(char *user, int flags, struct confinfo **matched);

/*
 * Login user on the current system.
 * Parameters are:
 *	userid		A Lyskom user id.
 *	passwd		The corresponding password.
 *
 * Return values:
 *	0		Login succeeded
 *	-1		Login failed
 */
int	rkom_login(int userid, char *passwd);

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
 * Asks the server for the current time.
 * This call never fails.
 */
void	rkom_time(struct tm *tm);

/*
 * Set the what-i-am-doing string for the logged-in user.
 * This call may fail with a normal Lyskom error code.
 */
int	rkom_whatido(char *str);

/*
 * Get the conference numbers with unread texts for a user.
 * Parameter is:
 *	uid		user id number
 *	confs		pointer to an array of conferences (retval)
 *	nconf		entries in the previous array (retval)
 *
 * Return values:
 *	A normal Lyskom error code, or 0.
 *
 * The array of integers contains the conferences with unread texts.
 * The pointer must be freed by the caller.
 */
int	rkom_unreadconf(int mid, int **confs, int *nconf);

/*
 * Get information about a conference.
 * Parameter is:
 *	mid		conference id number
 *	conf		pointer to a pointer to a struct conference
 *
 * Return values:
 *	A normal Lyskom error code, or 0.
 *
 *	The conference struct must be free'd after use.
 */
int	rkom_confinfo(int mid, struct conference **conf);

/*
 * Get information about a person.
 * Parameter is:
 *	uid		person id number
 *	person		pointer to a pointer to a struct person
 *
 * Return values:
 *	A normal Lyskom error code, or 0.
 *
 *	The person struct must be free'd after use.
 */
int	rkom_persinfo(int uid, struct person **person);

/*
 * Get status for a user in a conference.
 * Parameters are:
 *	uid		user id
 *	mid		conference id
 *	members		pointer to a pointer to a struct membership
 *
 * Return values:
 *	A normal Lyskom error code, or 0.
 *
 *	The membership struct must be free'd after use.
 */
int	rkom_membership(int uid, int conf, struct membership **members);

/*
 * Get info about who is logged on to the server.
 * Parameter is:
 *	info		pointer to a pointer to a struct dynamic_session_info
 *	secs		only persons active in this period of time
 *	flags		invisible and/or visible persons.
 *
 * The dynamic_session_info struct must be free'd after use.
 *
 * This call never fails.
 */
#define WHO_VISIBLE	1
#define WHO_INVISIBLE	2
void	rkom_who(int secs, int flags, struct dynamic_session_info **info);

/*
 * Async messages:
 *	
 */

/*
 * CONFERENCE STRINGS:
 *	TBD...
 */

