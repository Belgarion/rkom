




/*
 * Variables that can be altered in the config file, or on the command line.
 */
extern char *server;	/* Lyskom server to connect to (only in rc file) */
extern char *user;	/* User to log in as (only in rc file) */
extern char *pass;	/* Password to use when logging in (only in rc file) */

void parsefile(char *);	/* Reads and parses config file */
void readvars(void);
int iseql(char *var, char *val);
int isneq(char *var, char *val);
void set_flags(void);
void set_setflag(char *, char *);
void set_saveflags(void);
char *getval(char *);
