/*	$Id: backend.h,v 1.1 2000/09/26 18:48:00 ragge Exp $	*/
/*
 * Prototypes for the rkom backend internal functions.
 */

/* Lyskom protocol helper functions, in rkom_lyskom.c */
int	send_reply(char *msg);
char	get_char(void);
int	get_int(void);
void	get_eat(char c);
void	put_char(char c);
void	put_string(char *str);
void	get_accept(char ch);
char	*get_string(void);

/* Main loop, in rkom_be.c */
int	rkom_loop(void);
