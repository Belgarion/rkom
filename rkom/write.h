

void	write_new(char *);
void	write_editor(char *);
void	write_forget(char *);
void	write_rcpt(char *, int);
void	write_comment(char *);
void	write_footnote(void);
void	write_footnote_no(int);
void	write_cmnt(void);
void	write_cmnt_no(int);
void	write_cmnt_last(void);
void	write_put(char *);
void	write_whole(char *);
void	write_brev(char *);
void	write_private(int);
void	write_change_presentation(char *);

int	is_writing; /* Is writing text right now */
