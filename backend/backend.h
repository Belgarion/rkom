/*	$Id: backend.h,v 1.2 2000/10/01 14:10:41 ragge Exp $	*/
/*
 * Prototypes for the rkom backend internal functions.
 */

/* Forward declarations */
struct conference;
struct person;
struct membership;
struct tm;
struct aux_item;

/* Lyskom protocol helper functions, in rkom_lyskom.c */
int	send_reply(char *msg);
char	get_char(void);
int	get_int(void);
void	get_eat(char c);
void	put_char(char c);
void	put_string(char *str);
void	get_accept(char ch);
char	*get_string(void);

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

/* conference requests, in rkom_conf.c */
int	get_conf_stat(int confno, struct conference **confer);
int	get_pers_stat(int persno, struct person **pers);
int	get_membership(int uid, int conf, struct membership **member);

/* Helper functions, in rkom_helpers.c */
void	read_in_time(struct tm *t);
void	read_in_aux_item(struct aux_item *a);

/* variables */
extern	int readfd;	/* Get messages from frontend */
extern	int writefd;	/* Write to frontend */
extern	int myuid;	/* Current active uid */
