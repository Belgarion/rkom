

struct rk_time;

/*
 * Describes a command and which function to call if a match is found.
 * All commands are one or two words.
 */
struct cmnd {
	char *arg1;
	char *arg2;
	void (*func)(char *);
};

/* kbd.c */
int	kbd_input(int);
char *	get_input_string(int, int);

/* cmd.c */
void	cmd_parse(char *);

/* error.c */
char *	error(int);

/* show.c */
void	show_text(int);
void show_savetext(char *);
char * get_date_string(struct rk_time *t);
char *vem(int);

/* In file helpers.c */
struct rk_confinfo_retval *match_complain(char *str, int type);

extern	int myuid;
extern	int curconf;
extern	char *prompt, *p_see_time, *p_next_conf, *p_next_text;
extern	char *p_next_comment;
extern	int wrows; /* Rows per screen */

#define	PROMPT_SEE_TIME	p_see_time
#define	PROMPT_NEXT_CONF p_next_conf
#define	PROMPT_NEXT_TEXT p_next_text
#define	PROMPT_NEXT_COMMENT p_next_comment
