

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

int	kbd_input(int);
void	msg_input(int);
void	cmd_parse(char *);
int	send_reply(char *);
int	handle(void);
char *	error(int);
void	put_char(char);
void	put_string(char *);
int	async(int);
void	async_handle(int);
void	cmd_lista(char *);
void	cmd_nasta(char *);
void	cmd_login(char *);
void	show_text(int);
void	cmd_exec(char *, struct cmnd *);
char *	get_input_string(int, int);
char * get_date_string(struct rk_time *t);

/* In file helpers.c */
struct rk_confinfo_retval *match_complain(char *str, int type);

extern	int myuid;
extern	int curconf;
extern	char *prompt, *p_see_time, *p_next_conf, *p_next_text;
extern	char *p_next_comment;

#define	PROMPT_SEE_TIME	p_see_time
#define	PROMPT_NEXT_CONF p_next_conf
#define	PROMPT_NEXT_TEXT p_next_text
#define	PROMPT_NEXT_COMMENT p_next_comment
