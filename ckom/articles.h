/* $Id: articles.h,v 1.3 2000/10/15 19:33:34 jens Exp $ */
#ifndef ARTICLES_H
#define ARTICLES_H

#include "container.h"

#define	ART_UNREAD	((u_int32_t)1)
#define	ART_MARKED	((u_int32_t)1<<1)
#define	ART_FOOTN	((u_int32_t)1<<2)

typedef struct article art_t;
struct article {
	u_int32_t	art_id;
	u_int32_t	art_flags;
	u_int32_t	art_no_of_lines;
	char		art_subj[80];
	char		art_real_subj[80];
	char		art_from[80];
	int			art_comm_depth;
	time_t		art_time;
	art_t		*art_parent;
	char		**art_header;
	char		**art_footer;
	char		**art_text;
};

int art_open_conf(u_int32_t uid, u_int32_t conf_id);
art_t *art_get_num(int art_num);
char *art_get_conf_name(void);
int art_count(void);

#endif /* ARTICLES_H */
