




/*
 * Variables that can be altered in the config file, or on the command line.
 */
extern char *server;	/* Lyskom server to connect to (only in rc file) */
extern char *user;	/* User to log in as (only in rc file) */
extern char *pass;	/* Password to use when logging in (only in rc file) */
extern int use_editor;	/* Always use the editor $EDITOR */
extern int no_user_active; /* No "I'm alive"-messages to the server. */

void parsefile(char *);	/* Reads and parses config file */
