

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
char *	getstr(char *);
void	tcapinit(void);

/* cmd.c */
void	cmd_parse(char *);
void	cmd_only(char *);
void	cmd_password(void);
void	cmd_status(char *);
void	cmd_info_extra(int);
void	cmd_change_name(void);
void	cmd_add_member(void);
void	cmd_sub_member(void);
void	cmd_add_rcpt(void);
void	cmd_sub_rcpt(void);
void	cmd_delete(int);
void	cmd_erase(char *);
void	cmd_create(void);
void	cmd_create_person(void);
void	cmd_copy(void);
void	cmd_move_text(void);

/* error.c */
char *	error(int);

/* show.c */
int	show_text(int, int);
void show_savetext(char *);
char * get_date_string(struct rk_time *t);
char *vem(int);
void show_superhoppa(char *);

/* In file helpers.c */
struct rk_confinfo_retval *match_complain(char *str, int type);
int ismember(int conf);

/* rprintf.c */
void	rprintf(char const *fmt, ...);

/* parse.c */
void	chrconvert(char *str);

extern	int myuid;
extern	int curconf;
extern	char *prompt, *p_see_time, *p_next_conf, *p_next_text;
extern	char *p_next_comment;
extern	int wrows, wcols, outlines, swascii, discard; /* Rows per screen */
extern	char *client_version;

#define	PROMPT_SEE_TIME	p_see_time
#define	PROMPT_NEXT_CONF p_next_conf
#define	PROMPT_NEXT_TEXT p_next_text
#define	PROMPT_NEXT_COMMENT p_next_comment
