/*	$Id: backend.h,v 1.12 2002/09/04 12:38:02 ragge Exp $	*/
/*
 * Prototypes for the rkom backend internal functions.
 */

/* Forward declarations */
struct conference;
struct person;
struct membership;
struct tm;
struct rk_aux_item;

/* Lyskom protocol helper functions, in rkom_lyskom.c */
int	send_reply(char *msg);
char	get_char(void);
int	get_int(void);
void	get_eat(char c);
void	put_char(char c);
void	put_string(char *str);
void	get_accept(char ch);
char	*get_string(void);
void	send_callback(char *msg, int arg, void(*)(int, int));

/* Main loop, in rkom_subr.c */
int	rkom_loop(void);

/* Communication fe/be, in rkom_be.c */
int	fgrw(int cmd, void *arg, int arglen, void **reply, int *replylen);
void	bgreceive(void);
void	bgsend(int retval, int len, void *reply);
struct iovec;
void	bgsendv(int retval, int elem, struct iovec *iov);

/* Parsing of messages, in rkom_beparse.c */
void	rkom_beparse(int cmd, void *svar, int len);
void	reread_text_stat_bg(int text);
void	readin_textstat(struct rk_text_stat *ts);

/* conference requests, in rkom_conf.c */
int	get_conf_stat(int confno, struct rk_conference **confer);
int	get_pers_stat(int persno, struct rk_person **pers);
int	get_membership(int uid, int conf, struct rk_membership **member);
void	conf_set_high_local(int conf, int local, int global);
void	reread_conf_stat_bg(int conf);
void	newname(int);
void	invalidate_local(struct rk_text_stat *ts);

/* Helper functions, in rkom_helpers.c */
void	read_in_time(struct rk_time *t);
void	read_in_aux_item(struct rk_aux_item *a);

/* Async functions, in rkom_async.c */
void	async(int);
void	async_handle(void);

/* variables */
extern	int readfd;	/* Get messages from frontend */
extern	int writefd;	/* Write to frontend */
extern	int myuid;	/* Current active uid */
